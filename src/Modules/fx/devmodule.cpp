#include <Modules/BaseModule.h>
#include <Modules/Menu.h>
#include <Modules/Draw.h>
#include <Modules/fx/recorder.h>
#include <Hooks/OverlayHook.h>
#include <Streams/videowriter.h>
#include <Base/Interfaces.h>
#include <SDK/client_class.h>
#include <imgui/imgui_internal.h>
#include <Helper/str.h>
#include <array>
#include <vector>
#include <mutex>
#include <d3d9.h>
#include <Helper/dxerr.h>
#include <fstream>

#define PRINT_DXRESULT(expr) PrintDXResult(expr, #expr)

const uint32_t FOURCC_INTZ = MAKEFOURCC('I','N','T','Z');
bool PrintDXResult(HRESULT err, const char* expr);

class DevModule : public CModule {
protected:
    void StartListening() override;

private:
    bool m_buffers_initialized = false;
    bool m_dump_depth = false;
    std::array<char, MAX_PATH + 1> m_dump_filename = {"depth.png"};
    float m_depthpow = 90;
    D3DSURFACE_DESC m_depthstencil_desc;
    D3DSURFACE_DESC m_rendertarget_desc;
    IDirect3DSurface9* m_depthstencil;
    IDirect3DSurface9* m_rendertarget;
    IDirect3DSurface9* m_depthreplacement = nullptr;
    std::vector<std::pair<std::string, Vector>> m_3dmarkers;
    std::mutex m_3dmarkers_mutex;

    int OnMenu();
    int OnDraw();
    int OnReset();
    int OnPresent();

    static void DisplayPropertyTree(RecvProp* prop);
    static void DisplayTataTableTree(RecvTable* table);
    static void DisplaySurfaceInfo(const D3DSURFACE_DESC& desc, const char* label);
    IDirect3DSurface9* CreateTextureGetSurface(UINT Width,UINT Height,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool);
};

DevModule g_devmodule;

void DevModule::StartListening()
{
    Listen(EVENT_MENU, [this] { return OnMenu(); });
    Listen(EVENT_DRAW, [this] { return OnDraw(); });
    Listen(EVENT_DX9RESET, [this] { return OnReset(); });
    Listen(EVENT_DX9PRESENT, [this] { return OnPresent(); });
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

    m_dump_depth = ImGui::Button("Dump depth");
    ImGui::InputText("Dump file (PNG)", m_dump_filename.data(), m_dump_filename.size() - 1);
    ImGui::SliderFloat("Depth pow", &m_depthpow, 1, UINT8_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

    if (m_buffers_initialized)
    {
        DisplaySurfaceInfo(m_rendertarget_desc, "Render target");
        DisplaySurfaceInfo(m_depthstencil_desc, "Depth stencil");
    }

    if (!m_depthreplacement)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        if (m_rendertarget_desc.MultiSampleType != D3DMULTISAMPLE_NONE)
            ImGui::TextWrapped("Depth buffer is not available while anti-aliasing is enabled");
        else
        {
            ImGui::TextWrapped(
                "Your hardware does not support the 'INTZ' depth format.\n"
                "Depth buffer will be unavailable."
            );
        }
        ImGui::PopStyleColor();
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
            m_3dmarkers.emplace_back(std::make_pair(std::string(name.data()), Vector(pos[0], pos[1], pos[2])));
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

        if (m_rendertarget_desc.MultiSampleType == D3DMULTISAMPLE_NONE)
        {
            if (m_depthreplacement = CreateTextureGetSurface(
                m_depthstencil_desc.Width, m_depthstencil_desc.Height,
                D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)FOURCC_INTZ, D3DPOOL_DEFAULT)
            ) {
                g_hk_overlay.ReplaceDepthStencil(m_depthstencil, m_depthreplacement);
            } else
                printf("Failed to create replacement depth texture\n");
        }
        
        m_buffers_initialized = true;
    }

    if (m_dump_depth && m_buffers_initialized && m_depthreplacement)
    {
        m_dump_depth = false;

        D3DLOCKED_RECT rect;
        if (PRINT_DXRESULT(m_depthreplacement->LockRect(&rect, NULL, D3DLOCK_READONLY)))
        {
            FrameBufferRgb fb = {m_depthstencil_desc.Width, m_depthstencil_desc.Height};
            for (size_t y = 0; y < fb.GetHeight(); ++y)
            {
                for (size_t x = 0; x < fb.GetWidth(); ++x)
                {
                    // Remember to use `Pitch` from `D3DLOCKED_RECT`, as rects are usually rounded up to a power of 2
                    uint32_t* input = (uint32_t*)((uint8_t*)rect.pBits + x * 4 + y * rect.Pitch);
                    size_t offset = y * fb.GetWidth() + x;
                    uint8_t* output = fb.GetData() + offset * 3;

                    double scaled = (double)(input[0] & 0xFFFFFF) / 0xFFFFFF;
                    scaled = std::pow(scaled, (double)m_depthpow);
                    scaled *= 255;
                    if (scaled > 255)
                        scaled = 255;
                    output[0] = scaled;
                    output[1] = scaled;
                    output[2] = scaled;
                }
            }
            m_depthreplacement->UnlockRect();

            std::fstream file {m_dump_filename.data(), std::ios::out | std::ios::binary};
            if (file)
                ImageWriter::WritePNG(fb, file);
        }
    }

    return 0;
}

IDirect3DSurface9* DevModule::CreateTextureGetSurface(
        UINT Width,UINT Height,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool
) {
    IDirect3DTexture9* texture;
    IDirect3DSurface9* surface;
    if (PRINT_DXRESULT(g_hk_overlay.Device()->CreateTexture(Width, Height, 1,
        Usage, Format,
        Pool, &texture, NULL))
    ) {
        if (PRINT_DXRESULT(texture->GetSurfaceLevel(0, &surface)))
            return surface;
        else
            texture->Release();
    }
    return nullptr;
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
    DXGetErrorDescriptionW(err, err_desc, sizeof(err_desc) / sizeof(err_desc[0]));
    printf("%S (%S): %s\n", DXGetErrorStringW(err), err_desc, expr);
    return false;
}