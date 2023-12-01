#include "hlaeui.h"
#include <Modules/Menu.h>
#include <Hooks/ClientHook.h>
#include <Helper/engine.h>
#include <Helper/imgui.h>
#include <Base/Interfaces.h>
#include <SDK/cdll_int.h>
#include <chrono>

// Filter example: {L"C++ code files", L"*.cpp;*.h;*.rc"}
static const COMDLG_FILTERSPEC COM_CAMPATH_FILTER[] = {{L"HLAE campath file", L"*.xml"}, {0}};
static std::filesystem::path AUTOSAVE_PARENT_PATH;
static std::filesystem::path AUTOSAVE_PATH;
static std::string AUTOSAVE_PATH_UTF8;

void HlaeUi::StartListening()
{
    AUTOSAVE_PARENT_PATH = Base::GetModuleDir() / "sparklyfx" / "autosave";
    AUTOSAVE_PATH = AUTOSAVE_PARENT_PATH / "hlae_campath.xml";
    AUTOSAVE_PATH_UTF8 = AUTOSAVE_PATH.u8string();

    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int HlaeUi::OnMenu()
{
    if (!ImGui::CollapsingHeader("HLAE (Half-Life Advanced Effects)"))
        return 0;

    // Shadow the member variable so we don't make any mistakes due to threading
    bool _is_hlae_loaded = m_is_hlae_loaded;
    bool m_is_hlae_loaded = _is_hlae_loaded;

    if (!m_is_hlae_loaded)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        ImGui::TextWrapped("HLAE is not loaded. Visit advancedfx.org to learn about HLAE.");
        ImGui::PopStyleColor();
        ImGui::BeginDisabled();
    }

    if (ImGui::BeginTabBar("hlae_tabs"))
    {
        if (ImGui::BeginTabItem("Cam path"))
        {
            if (ImGui::Checkbox("Draw path", &m_campaths_draw))
                Helper::ClientCmd_Unrestricted("mirv_campath draw enabled %d", (int)m_campaths_draw);
            if (ImGui::Button("Add path"))
                Interfaces::engine->ClientCmd_Unrestricted("mirv_campath add");

            if (ImGui::Button("Save..."))
                SaveCamPath();
            ImGui::SameLine();
            if (ImGui::Button("Save as..."))
                SaveCamPath(true);
            
            if (ImGui::Button("Load..."))
                LoadCamPath(nullptr);
            ImGui::SameLine();
            if (ImGui::Button("Load autosave"))
                LoadCamPath(&AUTOSAVE_PATH);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Autosave"))
        {
            ImGui::Checkbox("Autosave", &m_autosave); ImGui::SameLine();
            Helper::ImGuiHelpMarker("Make periodic saves of the current campath");
            if (m_autosave)
            {
                int new_duration = m_autosave_mins;
                if (ImGui::InputInt("Interval (mins)", &new_duration))
                {
                    if (new_duration < 1)
                        new_duration = 1;
                    else if (new_duration > 60)
                        new_duration = 60;
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (!m_is_hlae_loaded)
        ImGui::EndDisabled();
    return 0;
}

int HlaeUi::OnFrameStageNotify()
{
    if (!m_is_hlae_loaded)
    {
        FindCommands();
        return 0;
    }

    if (g_hk_client.Context()->curStage != FRAME_START)
        return 0;
    
    static auto prev_time = std::chrono::steady_clock::now();
    auto cur_time = std::chrono::steady_clock::now();
    auto elapsed_mins = std::chrono::duration_cast<std::chrono::minutes>(cur_time - prev_time).count();

    if (elapsed_mins >= m_autosave_mins)
    {
        prev_time = cur_time;
        if (m_autosave)
            WriteNewAutosave();
    }

    return 0;
}

void HlaeUi::FindCommands()
{
    if (m_is_hlae_loaded)
        return;
    if (Interfaces::cvar->FindCommand("mirv_campath"))
        m_is_hlae_loaded = true;
}

void HlaeUi::SaveCamPath(bool save_as)
{
    if (save_as || !m_prev_campath_save)
    {
        m_prev_campath_save = Helper::SaveFileDialog(L"Save cam path", nullptr, COM_CAMPATH_FILTER);
        if (!m_prev_campath_save)
            return;
    }

    // u8string will convert to UTF-8
    auto u8str = m_prev_campath_save->u8string();
    Helper::ClientCmd_Unrestricted("mirv_campath save \"%.*s\"", (int)u8str.length(), (const char*)u8str.c_str());
}

void HlaeUi::LoadCamPath(const std::filesystem::path* path)
{
    std::optional<std::filesystem::path> selected_path;
    if (path == nullptr)
    {
        selected_path = Helper::OpenFileDialog(L"Load cam path", nullptr, COM_CAMPATH_FILTER);
        if (!selected_path)
            return;
        path = &selected_path.value();
    }

    auto u8str = path->u8string();
    Helper::ClientCmd_Unrestricted("mirv_campath load \"%.*s\"", (int)u8str.length(), (const char*)u8str.c_str());
}

void HlaeUi::WriteNewAutosave()
{
    std::error_code err;
    std::filesystem::create_directories(AUTOSAVE_PARENT_PATH, err);
    if (!err)
        Helper::ClientCmd_Unrestricted("mirv_campath save \"%.*s\"", AUTOSAVE_PATH_UTF8.length(), AUTOSAVE_PATH_UTF8.c_str());
}