#include <Modules/BaseModule.h>
#include "mainwindow.h"
#include <Modules/Draw.h>
#include <Hooks/OverlayHook.h>
#include <Base/Interfaces.h>
#include <SDK/client_class.h>
#include <imgui/imgui_internal.h>
#include <Helper/str.h>
#include <array>
#include <vector>
#include <mutex>
#include <d3d9.h>
#include <Helper/dxerr.h>
#include <Helper/imgui.h>

#define PRINT_DXRESULT(expr) PrintDXResult(expr, #expr)

bool PrintDXResult(HRESULT err, const char* expr);

class DevModule : public CModule {
protected:
    void StartListening() override;

private:
    bool m_buffers_initialized = false;
    D3DSURFACE_DESC m_depthstencil_desc;
    D3DSURFACE_DESC m_rendertarget_desc;
    IDirect3DSurface9* m_depthstencil;
    IDirect3DSurface9* m_rendertarget;
    std::vector<std::pair<std::string, Vector>> m_3dmarkers;
    std::mutex m_3dmarkers_mutex;

    int OnMenu();
    int OnDraw();
    int OnReset();
    int OnPresent();


    static void DisplayPropertyTree(RecvProp* prop);
    static void DisplayTataTableTree(RecvTable* table);
    static void DisplaySurfaceInfo(const D3DSURFACE_DESC& desc, const char* label);
};

DevModule g_devmodule;

void DevModule::StartListening()
{
    MainWindow::OnWindow.Listen(&DevModule::OnMenu, this);
    CDraw::OnDraw.Listen(&DevModule::OnDraw, this);
    COverlayHook::OnReset.ListenNoArgs(&DevModule::OnReset, this);
    COverlayHook::OnPresent.ListenNoArgs(&DevModule::OnPresent, this);
}

void DevModule::DisplayPropertyTree(RecvProp* prop)
{
    if (prop->GetType() == SendPropType::DPT_DataTable)
        DisplayTataTableTree(prop->GetDataTable());
    else
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        window->DC.CursorPos.x += ImGui::GetTreeNodeToLabelSpacing();
        ImGui::Text("%s", prop->GetName());
    }
}

void DevModule::DisplayTataTableTree(RecvTable* table)
{
    if (ImGui::TreeNode(table->GetName()))
    {
        for (size_t i = 0; i < table->m_nProps; ++i)
            DisplayPropertyTree(&table->m_pProps[i]);
        ImGui::TreePop();
    }
}

int DevModule::OnMenu()
{
    const char* const POPUP_3DMARKER = "##popup_3dmarker";
    
    if (!ImGui::CollapsingHeader("Dev tools"))
        return 0;
    
    static std::array<char, 64> filter_text = {0};
    ImGui::InputText("Class filter", filter_text.data(), filter_text.size() - 1);

    if (ImGui::TreeNode("Classes"))
    {
        ClientClass* next_class = Interfaces::hlclient->GetAllClasses();
        for (; next_class; next_class = next_class->m_pNext)
        {
            if (filter_text[0] && !Helper::strcasestr(next_class->m_pNetworkName, filter_text.data()))
                continue;
            
            if (ImGui::TreeNode(next_class->m_pNetworkName))
            {
                DisplayTataTableTree(next_class->m_pRecvTable);
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    if (m_buffers_initialized)
    {
        DisplaySurfaceInfo(m_rendertarget_desc, "Render target");
        DisplaySurfaceInfo(m_depthstencil_desc, "Depth stencil");
    }
    
    static size_t selected_marker = 0;

    if (ImGui::Button("Add##marker"))
        ImGui::OpenPopup(POPUP_3DMARKER);
    ImGui::SameLine();
    if (ImGui::Button("Remove##marker"))
    {
        std::scoped_lock lock(m_3dmarkers_mutex);
        if (selected_marker < m_3dmarkers.size())
            m_3dmarkers.erase(m_3dmarkers.begin() + selected_marker);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear##marker"))
    {
        std::scoped_lock lock(m_3dmarkers_mutex);
        m_3dmarkers.clear();
    }

    if (ImGui::BeginListBox("Markers", Helper::CalcListBoxSize(m_3dmarkers.size())))
    {
        std::scoped_lock lock(m_3dmarkers_mutex);
        size_t i = 0;
        for (auto& pair : m_3dmarkers)
        {
            ImGui::PushID(i);
            if (ImGui::Selectable(pair.first.c_str(), i == selected_marker))
                selected_marker = i;
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginPopup(POPUP_3DMARKER))
    {
        static float pos[3] = {0};
        static std::array<char, 64> name = {0};

        ImGui::InputText("Name", name.data(), name.size() - 1);
        ImGui::InputFloat3("Pos", pos);

        if (ImGui::Button("Add") && name[0])
        {
            std::scoped_lock lock(m_3dmarkers_mutex);
            m_3dmarkers.emplace_back(std::string(name.data()), Vector(pos[0], pos[1], pos[2]));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return 0;
}

int DevModule::OnDraw()
{
    {
        std::scoped_lock lock(m_3dmarkers_mutex);
        for (auto& pair : m_3dmarkers)
        {
            Vector world_pos = pair.second;
            ImVec2 screen_pos;
            if (!gDraw.WorldToScreen(world_pos, screen_pos))
                continue;
            gDraw.DrawText_Outline(screen_pos, 0xFFFFFFFF, 0xFF000000, pair.first.c_str());
            gDraw.DrawBox3D_Radius(world_pos, 50, ImColor(255, 0, 0));
        }
    }
    return 0;
}

int DevModule::OnReset()
{
    m_buffers_initialized = false;
    return 0;
}

int DevModule::OnPresent()
{
    IDirect3DDevice9* device = g_hk_overlay.Device();

    if (!m_buffers_initialized)
    {
        if (!PRINT_DXRESULT(device->GetDepthStencilSurface(&m_depthstencil)))
            return 0;
        if (!PRINT_DXRESULT(device->GetRenderTarget(0, &m_rendertarget)))
            return 0;

        m_depthstencil->GetDesc(&m_depthstencil_desc);
        m_rendertarget->GetDesc(&m_rendertarget_desc);
        m_buffers_initialized = true;
    }

    return 0;
}

void DevModule::DisplaySurfaceInfo(const D3DSURFACE_DESC& desc, const char* label)
{
    if (ImGui::TreeNode(label))
    {
        ImGui::Text("Format: %d", (int)desc.Format);
        ImGui::Text("Type:   %d", (int)desc.Type);
        ImGui::Text("Usage:  %d", (int)desc.Usage);
        ImGui::Text("Pool:   %d", (int)desc.Pool);
        ImGui::Text("MultiSampleType: %d", (int)desc.MultiSampleType);
        ImGui::Text("MultiSampleQuality: %d", (int)desc.MultiSampleQuality);
        ImGui::Text("Width:  %d", (int)desc.Width);
        ImGui::Text("Height: %d", (int)desc.Height);
        ImGui::TreePop();
    }
}

static bool PrintDXResult(HRESULT err, const char* expr)
{
    if (SUCCEEDED(err))
        return true;
    
    WCHAR err_desc[512];
    DXGetErrorDescriptionW(err, err_desc, std::size(err_desc));
    printf("%S (%S): %s\n", DXGetErrorStringW(err), err_desc, expr);
    return false;
}