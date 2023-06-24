#include "RenderConfigEditor.h"
#include "ActiveRenderConfig.h"
#include <Modules/Menu.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <array>

static const char* POPUP_FRAME_CREATOR = "##popup_frame_creator";
static const char* POPUP_FRAME_EDITOR = "##popup_frame_editor";
static const char* POPUP_TWEAK_CREATOR = "##popup_tweak_creator";
static const char* POPUP_TWEAK_EDITOR = "##popup_tweak_editor";

void RenderConfigEditor::StartListening() {
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
}

int RenderConfigEditor::OnMenu()
{
    if (ImGui::CollapsingHeader("Streams"))
    {
        ImGui::BeginGroup();
        ShowConfigListEditor();
        ImGui::EndGroup();
    }

    return 0;
}

void RenderConfigEditor::ShowConfigListEditor()
{
    static auto frame_getter = [](void* data, int index, const char** out_text)
    {
        auto frames_vec = (const decltype(RenderConfigEditor::m_render_configs)*)data;
        if (index >= 0 && index < frames_vec->size())
        {
            *out_text = frames_vec->at(index)->GetName().c_str();
            return true;
        }
        return false;
    };

    static int current_cfg = 0;

    {
        ImGui::BeginGroup();

        ImGui::SeparatorText("Stream list");
        
        if (ImGui::Button("Add##frame"))
        ImGui::OpenPopup(POPUP_FRAME_CREATOR);
        ImGui::SameLine();
        if (ImGui::Button("Remove##frame"))
        {
            if (current_cfg >= 0 && current_cfg < m_render_configs.size())
                m_render_configs.erase(m_render_configs.begin() + current_cfg);
        }

        bool change_selection = ImGui::ListBox("##frames_list", &current_cfg, frame_getter, &m_render_configs, m_render_configs.size());
        bool change_preview = ImGui::Checkbox("Preview the selected stream", &m_preview);

        if (change_selection || change_preview)
        {
            if (!m_preview)
                g_active_rendercfg.SetNone();
            else
            {
                if (current_cfg >= 0 && current_cfg < m_render_configs.size())
                    g_active_rendercfg.Set(m_render_configs.at(current_cfg));
            }
        }

        ImGui::EndGroup();
    }

    if (current_cfg >= 0 && current_cfg < m_render_configs.size())
    {
        ImGui::BeginGroup();

        RenderConfig::Ptr render_config = m_render_configs.at(current_cfg);
        ImGui::SeparatorText(render_config->GetName().c_str());
        ShowConfigEditor(render_config);

        ImGui::EndGroup();
    }

    PopupConfigCreator();
}

void RenderConfigEditor::PopupConfigCreator()
{
    if (ImGui::BeginPopup(POPUP_FRAME_CREATOR))
    {
        static std::array<char, 64> input_frame_name = { "my stream" };
        ImGui::Text("Choose a name");
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##new_frame_name", input_frame_name.data(), input_frame_name.size());
        ImGui::PopItemWidth();
        if (ImGui::Button("Ok##new_frame")) {
            m_render_configs.emplace_back(std::make_shared<RenderConfig>(input_frame_name.data()));
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RenderConfigEditor::ShowConfigEditor(RenderConfig::Ptr render_config)
{
    auto frame_tweaks = &render_config->GetRenderTweaks();
    
    static auto tweaks_getter = [](void* data, int index, const char** out_text)
    {
        auto tweaks = (decltype(frame_tweaks))data;
        if (index < 0 || index >= tweaks->size())
            return false;
        *out_text = tweaks->at(index)->GetName();
        return true;
    };

    const float child_height = 200;
    static int current_tweak = -1;
    {
        ImGui::BeginChild("##frame_editor", ImVec2(200, child_height));

        PopupTweakCreator(render_config);
        ImGui::InputText("Name##frame", &render_config->GetName());
        ImGui::Text("Active tweaks");
        if (ImGui::Button("Add##tweak"))
            ImGui::OpenPopup(POPUP_TWEAK_CREATOR);
        ImGui::SameLine();
        if (ImGui::Button("Remove##tweak"))
        {
            if (current_tweak >= 0 && current_tweak < frame_tweaks->size())
                frame_tweaks->erase(frame_tweaks->begin() + current_tweak);
        }

        ImGui::SetNextItemWidth(-1);
        ImGui::ListBox("##Active tweaks", &current_tweak, tweaks_getter, frame_tweaks, frame_tweaks->size());
        ImGui::EndChild();
    }

    if (current_tweak >= 0 && current_tweak < frame_tweaks->size())
    {
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5);
        ImGui::BeginChild("##tweak_editor", ImVec2(0, child_height), true);
        ShowTweakEditor(frame_tweaks->at(current_tweak));
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

void RenderConfigEditor::PopupTweakCreator(RenderConfig::Ptr render_config)
{
    static auto default_tweaks_getter = [](void* data, int index, const char** out_text)
    {
        if (index < 0 || index >= RenderTweak::default_tweaks.size())
            return false;
        *out_text = RenderTweak::default_tweaks.at(index)->GetName();
        return true;
    };

    if (ImGui::BeginPopup(POPUP_TWEAK_CREATOR))
    {
        ImGui::Text("Choose a tweak");
        // TODO: Manually use Begin/End List, because it will be faster and storing the last selected state is useless
        int choice = -1;
        bool ok = ImGui::ListBox("##default_tweaks_list", &choice, default_tweaks_getter, nullptr, RenderTweak::default_tweaks.size());
        bool cancel = ImGui::Button("Cancel##new_tweak");

        if (ok)
        {
            if (choice >= 0 && choice < RenderTweak::default_tweaks.size())
                render_config->GetRenderTweaks().emplace_back(RenderTweak::default_tweaks.at(choice)->Clone());
        }

        if (ok || cancel)
                ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }
}

void RenderConfigEditor::ShowTweakEditor(RenderTweak::Ptr render_tweak) {
    render_tweak->OnMenu();
}
