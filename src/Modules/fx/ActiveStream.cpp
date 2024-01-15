#include "ActiveStream.h"
#include <imgui/backends/imgui_impl_dx9.h>
#include <Shaders/shaders.h>
#include <Streams/materials.h>
#include <Helper/engine.h>
#include <Helper/str.h>
#include <Helper/d3d9.h>
#include <Helper/defer.h>
#include <Hooks/OverlayHook.h>
#include <Hooks/fx/StudioRenderHook.h>
#include <Hooks/fx/ModelRenderHook.h>
#include <Hooks/fx/RenderViewHook.h>
#include <Hooks/ClientHook.h>
#include <Modules/fx/recorder.h>
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <SDK/ivrenderview.h>
#include <SDK/client_class.h>
#include <SDK/cdll_int.h>
#include <SDK/view_shared.h>
#include <SDK/icvar.h>
#include <SDK/convar.h>
#include <SDK/IPanel.h>

void ActiveStream::StartListening()
{
    Listen(EVENT_DRAWMODELSTATICPROP, [this]{ return OnDrawStaticProp(); });
    Listen(EVENT_PRE_DRAW_MODEL_EXECUTE, [this] { return PreDrawModelExecute(); });
    Listen(EVENT_POST_DRAW_MODEL_EXECUTE, [this] { return PostDrawModelExecute(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this] { return OnFrameStageNotify(); });
    Listen(EVENT_OVERRIDEVIEW, [this] { return OnOverrideView(); });
    Listen(EVENT_VIEW_DRAW_FADE, [this] { return OnViewDrawFade(); });
    Listen(EVENT_DX9PRESENT, [this] { return OnPresent(); });
    Listen(EVENT_DX9RESET, [this] { return OnReset(); });
}

Stream::Ptr ActiveStream::Get()
{
    auto lock = ReadLock();
    return m_stream;
}

void ActiveStream::Set(Stream::Ptr stream)
{
    auto lock = WriteLock();
    m_should_update_materials = m_stream != stream;
    m_stream = stream;
    UpdateFog();
    UpdateConVars();
    UpdateHud();
    UpdateRenderTarget();
}

void ActiveStream::SignalUpdate(Stream::Ptr stream, uint32_t flags)
{
    auto lock = WriteLock();
    if (m_stream == nullptr)
        return;
    if (stream == nullptr || m_stream == stream)
    {
        m_should_update_materials |= (bool)(flags & UPDATE_MATERIALS);
        UpdateHud();
        if (flags & UPDATE_FOG)
            UpdateFog();
        if (flags & UPDATE_CONVARS)
            UpdateConVars();
    }
}

bool ActiveStream::DrawDepth()
{
    IDirect3DDevice9* device = g_hk_overlay.Device();
    
    // Find a pixel shader to use
    Shader::PixelShader::Ptr pixel_shader = nullptr;
    if (IsDepthAvailable())
    {
        auto lock = ReadLock();
        if (m_stream)
        {
            for (auto it = m_stream->begin<CameraTweak>(); it != m_stream->end<CameraTweak>(); ++it)
            {
                if (it->pixel_shader)
                {
                    pixel_shader = it->pixel_shader;
                    break;
                }
            }
        }
    }

    if (!pixel_shader)
        return false;

    D3DSURFACE_DESC surface_desc;
    m_color->GetDesc(&surface_desc);

    // Setup DirectX state to draw a rectangle covering the screen.
    // The pixel shader will affect all pixels covered by the rectangle.
    // The DirectX state is restored at the end-of-scope.
    {
        IDirect3DStateBlock9* prev_state;
        device->CreateStateBlock(D3DSBT_ALL, &prev_state);
        defer {
            prev_state->Apply();
            prev_state->Release();
        };

        // Calculate the inverse projection matrix
        D3DMATRIX inverse_projection;
        {
            std::scoped_lock lock{m_matrices_mutex};
            Helper::Invert4x4Matrix(&m_projection_matrix.m[0][0], &inverse_projection.m[0][0]);
        }

        device->SetPixelShader(pixel_shader->GetDxPtr());
        device->SetDepthStencilSurface(nullptr);
        pixel_shader->ApplyConstants(&inverse_projection.m[0][0], m_depth_replacement_texture);

        Helper::DrawTexturedRect(device, {0, 0, (float)surface_desc.Width, (float)surface_desc.Height});
    }

    // Re-render ImGui because we probably drew over the menu...
    // We wouldn't have to re-render if this always ran before the ImGui code.
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (m_color_replacement != m_color)
    {
        device->StretchRect(
            m_color_replacement, nullptr,
            m_color, nullptr,
            D3DTEXF_NONE
        );
    }

    return true;
}

void ActiveStream::UpdateMaterials()
{
    auto lock = ReadLock();
    m_should_update_materials = false;
    
    for (   MaterialHandle_t handle = Interfaces::mat_system->FirstMaterial();
            handle != Interfaces::mat_system->InvalidMaterial();
            handle = Interfaces::mat_system->NextMaterial(handle)
        )
    {
        IMaterial* mat = Interfaces::mat_system->GetMaterial(handle);
        bool was_affected = false;
        if (m_stream)
        {
            for (auto tweak = m_stream->begin<MaterialTweak>(); tweak != m_stream->end<MaterialTweak>(); ++tweak)
            {
                if (tweak->IsMaterialAffected(mat))
                {
                    was_affected = true;
                    SetMaterialColor(mat, tweak->color_multiply);
                }
            }
        }
        if (!was_affected)
            RestoreMaterialParams(mat);
    }
}

void ActiveStream::UpdateConVars()
{
    for (auto& convar : m_convars)
        convar.Restore();
    m_convars.clear();

    if (!m_stream)
        return;

    auto tweak = m_stream->begin<CommandTweak>();
    if (tweak == m_stream->end<CommandTweak>())
        return;
    
    std::vector<Helper::ParsedCommand> parsed_cmds;
    tweak->GetCommandList(&parsed_cmds);

    for (auto& cmd : parsed_cmds)
    {
        std::string cmd_name = std::string(cmd.name);
        ConVar* cvar = Interfaces::cvar->FindVar(cmd_name.c_str());
        if (!cvar)
        {
            // It's probably a command. Dispatch the command the lazy way.
            cmd_name += ' ';
            cmd_name += cmd.args;
            Interfaces::engine->ExecuteClientCmd(cmd_name.c_str());
            continue;
        }
        
        std::string args = std::string(cmd.args);
        Helper::RestoringConVar restoring_cvar {cvar};
        restoring_cvar.SetValue(args.c_str());
        m_convars.push_back(std::move(restoring_cvar));
    }
}

void ActiveStream::UpdateFog()
{
    static ConVar* fog_override = Interfaces::cvar->FindVar("fog_override");
    static ConVar* fog_enable = Interfaces::cvar->FindVar("fog_enable");
    static ConVar* fog_start = Interfaces::cvar->FindVar("fog_start");
    static ConVar* fog_end = Interfaces::cvar->FindVar("fog_end");
    static ConVar* fog_color = Interfaces::cvar->FindVar("fog_color");

    static ConVar* skyfog_enable = Interfaces::cvar->FindVar("fog_enableskybox");
    static ConVar* skyfog_start = Interfaces::cvar->FindVar("fog_startskybox");
    static ConVar* skyfog_end = Interfaces::cvar->FindVar("fog_endskybox");
    static ConVar* skyfog_color = Interfaces::cvar->FindVar("fog_colorskybox");

    if (!m_stream)
    {
        fog_override->SetValue(0);
        return;
    }
    
    auto tweak = m_stream->begin<FogTweak>();
    if (tweak == m_stream->end<FogTweak>())
    {
        fog_override->SetValue(0);
        return;
    }
    fog_override->SetValue(1);

    std::array<char, 16> textbuf;
    
    sprintf_s(textbuf.data(), textbuf.size(), "%d %d %d",
        (int)(tweak->fog_color[0] * 255), (int)(tweak->fog_color[1] * 255), (int)(tweak->fog_color[2] * 255)
    );
    fog_enable->SetValue(tweak->fog_enabled);
    fog_start->SetValue(tweak->fog_start);
    fog_end->SetValue(tweak->fog_end);
    fog_color->SetValue(textbuf.data());

    sprintf_s(textbuf.data(), textbuf.size(), "%d %d %d",
        (int)(tweak->skyfog_color[0] * 255), (int)(tweak->skyfog_color[1] * 255), (int)(tweak->skyfog_color[2] * 255)
    );
    skyfog_enable->SetValue(tweak->skyfog_enabled);
    skyfog_start->SetValue(tweak->skyfog_start);
    skyfog_end->SetValue(tweak->skyfog_end);
    skyfog_color->SetValue(textbuf.data());
}

int ActiveStream::OnDrawStaticProp()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;

    auto tweak = m_stream->begin<MaterialTweak>();
    if (tweak == m_stream->end<MaterialTweak>())
        return 0;
    
    if (!tweak->props)
        return 0;
    if (tweak->color_multiply[3] == 0)
        return Return_NoOriginal;
    
    // We don't need to restore these values, because the callers always store and restore it.
    Interfaces::studio_render->SetColorModulation(tweak->color_multiply.data());
    Interfaces::studio_render->SetAlphaModulation(tweak->color_multiply[3]);
    return 0;
}

int ActiveStream::PreDrawModelExecute()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;
    
    auto* ctx = &g_hk_model_render.Context()->model_execute;
    CBaseEntity* entity = Interfaces::entlist->GetClientEntity(ctx->pInfo->entity_index);

    for (auto tweak = m_stream->begin<ModelTweak>(); tweak != m_stream->end<ModelTweak>(); ++tweak)
    {
        bool is_affected = entity && tweak->IsEntityAffected(entity);
        if (!is_affected)
            is_affected = tweak->IsModelAffected(ctx->state->m_pStudioHdr->pszName());

        if (!is_affected)
            continue;
        
        if (tweak->IsEffectInvisible())
            return Return_NoOriginal;

        // Store original draw parameters
        m_is_dme_affected = true;
        m_last_dme_params.mat_override = nullptr;
        m_last_dme_params.mat_override_type = OVERRIDE_NORMAL;
        Interfaces::render_view->GetColorModulation(m_last_dme_params.color.data());
        m_last_dme_params.color[3] = Interfaces::render_view->GetBlend();
        
        Interfaces::render_view->SetColorModulation(tweak->color_multiply.data());
        Interfaces::render_view->SetBlend(tweak->color_multiply[3]);

        if (tweak->custom_material)
            Interfaces::model_render->ForcedMaterialOverride(tweak->custom_material->GetMaterial());
    }
    return 0;
}

int ActiveStream::PostDrawModelExecute()
{
    auto lock = ReadLock();

    if (m_is_dme_affected)
    {
        Interfaces::model_render->ForcedMaterialOverride(m_last_dme_params.mat_override, m_last_dme_params.mat_override_type);
        Interfaces::render_view->SetColorModulation(m_last_dme_params.color.data());
        Interfaces::render_view->SetBlend(m_last_dme_params.color[3]);
    }

    m_is_dme_affected = false;
    return 0;
}

int ActiveStream::OnFrameStageNotify()
{
    ClientFrameStage_t stage = g_hk_client.Context()->curStage;
    if (stage != FRAME_RENDER_START)
        return 0;
        
    UpdateViewMatricies();

    // Everything from this point needs to read the active stream. Acquire the reading lock.
    auto lock = ReadLock();
    UpdateRenderTarget();
    if (m_should_update_materials)
        UpdateMaterials();

    // Always call this so that commands like `cl_drawhud` don't override our changes
    UpdateHud();

    return 0;
}

void ActiveStream::UpdateViewMatricies()
{
    CViewSetup view;
    Interfaces::hlclient->GetPlayerView(view);

    VMatrix world_view;
    VMatrix world_projection;
    VMatrix world_to_pixels;
    
    std::scoped_lock lock{m_matrices_mutex};
    Interfaces::render_view->GetMatricesForView(view, &world_view, (VMatrix*)&m_projection_matrix, &world_projection, &world_to_pixels);
}

void ActiveStream::UpdateRenderTarget()
{
    if (!m_color || !m_depth)
        return;
    
    IDirect3DSurface9* new_color = m_color;
    IDirect3DSurface9* new_depth = m_depth;

    if (IsDepthAvailable() && m_stream)
    {
        for (auto it = m_stream->begin<CameraTweak>(); it != m_stream->end<CameraTweak>(); ++it)
        {
            if (it->pixel_shader)
            {
                new_color = m_color_replacement;
                new_depth = m_depth_replacement_surface;
                break;
            }
        }
    }

    if (new_color == m_color)
        g_hk_overlay.RestoreRenderTarget(m_color);
    else
    {
        g_hk_overlay.ReplaceRenderTarget(m_color, new_color);
        g_hk_overlay.SetRenderTarget(0, new_color);
    }

    if (new_depth == m_depth)
        g_hk_overlay.RestoreDepthStencil(m_depth);
    else
    {
        g_hk_overlay.ReplaceDepthStencil(m_depth, new_depth);
        g_hk_overlay.SetDepthStencilSurface(new_depth);
    }
}

void ActiveStream::UpdateHud()
{
    if (!m_stream)
        return;

    // Show/hide the HUD

    class PanelPlaceholder
    {
    public:
        virtual vgui::VPANEL GetVPanel() = 0;
    };

    vgui::VPANEL viewport = ((PanelPlaceholder*)Interfaces::client->GetViewport())->GetVPanel();
    static ConVar* cl_drawhud = Interfaces::cvar->FindVar("cl_drawhud");
    bool is_visible = Interfaces::panels->IsVisible(viewport);
    bool new_visibility = cl_drawhud->GetBool();

    auto cam_tweak = m_stream->begin<CameraTweak>();
    if (cam_tweak != m_stream->end<CameraTweak>() && cam_tweak->hud != CameraTweak::HUD_DEFAULT)
        new_visibility = cam_tweak->hud == CameraTweak::HUD_ENABLED;
    
    if (is_visible != new_visibility)
        Interfaces::panels->SetVisible(viewport, new_visibility);
}

int ActiveStream::OnOverrideView()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;
    
    auto view_setup = g_hk_client.Context()->pSetup;
    for (auto tweak = m_stream->begin<CameraTweak>(); tweak != m_stream->end<CameraTweak>(); ++tweak)
    {
        if (tweak->fov_override)
            view_setup->fov = tweak->fov;
    }
    return 0;
}

int ActiveStream::OnViewDrawFade()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;
    
    for (auto tweak = m_stream->begin<CameraTweak>(); tweak != m_stream->end<CameraTweak>(); ++tweak)
    {
        if (tweak->hide_fade)
            return Return_NoOriginal;
    }
    return 0;
}

int ActiveStream::OnPresent()
{
    if (m_dx_reset)
    {
        InitializeDxStuff();
        m_dx_reset = false;
    }

    if (!g_recorder.IsRecordingMovie()) // If we're recording, then DrawDepth was already called.
        DrawDepth();
    return 0;
}

int ActiveStream::OnReset()
{
    // Remove our color and depth ptrs from the replacement map
    if (m_color)
        g_hk_overlay.RestoreRenderTarget(m_color);
    if (m_depth)
        g_hk_overlay.RestoreDepthStencil(m_depth);
    
    m_color = nullptr;
    m_depth = nullptr;
    m_color_replacement = nullptr;
    m_depth_replacement_surface = nullptr;
    m_depth_replacement_texture = nullptr;
    m_dx_reset = true;
    return 0;
}

void ActiveStream::StoreMaterialParams(IMaterial* mat)
{
    auto it = m_affected_materials.find(mat);
    if (it == m_affected_materials.end()) // Material will be added
        it = m_affected_materials.insert(std::pair<IMaterial*, OldMaterialParams>(mat, {})).first;
    else // Material will be validated
    {
        const auto& old_params = it->second;
        if (mat->GetName() == old_params.name)
            return; // The material already exists
    }
    
    OldMaterialParams& params = it->second;
    params.name = mat->GetName();
    mat->GetColorModulation(&params.color[0], &params.color[1], &params.color[2]);
    params.color[3] = mat->GetAlphaModulation();
}

void ActiveStream::RestoreMaterialParams(IMaterial* mat)
{
    auto it = m_affected_materials.find(mat);
    if (it == m_affected_materials.end())
        return;

    const auto& old_params = it->second;
    if (mat->GetName() == old_params.name)
    {
        mat->ColorModulate(old_params.color[0], old_params.color[1], old_params.color[2]);
        mat->AlphaModulate(old_params.color[3]);
    }
    
    m_affected_materials.erase(it);
}

void ActiveStream::SetMaterialColor(IMaterial* mat, const std::array<float, 4>& col)
{
    StoreMaterialParams(mat);
    mat->ColorModulate(col[0], col[1], col[2]);
    mat->AlphaModulate(col[3]);
}

void ActiveStream::InitializeDxStuff()
{
    IDirect3DDevice9* device = g_hk_overlay.Device();

    // Get the render target and depth buffer descriptions

    D3DSURFACE_DESC color_desc, depth_desc;
    
    if (FAILED(device->GetRenderTarget(0, &m_color))
        || FAILED(m_color->GetDesc(&color_desc))
    ) {
        printf("ActiveStream failed to get render target\n");
        return;
    }
    if (FAILED(device->GetDepthStencilSurface(&m_depth))
        || FAILED(m_depth->GetDesc(&depth_desc))
    ) {
        printf("ActiveStream failed to get depth stencil\n");
        return;
    }
    
    // Create the replacement render target depth buffer
    
    if (FAILED(device->CreateTexture(
        depth_desc.Width, depth_desc.Height, 1,
        D3DUSAGE_DEPTHSTENCIL, Helper::FOURCC_INTZ, D3DPOOL_DEFAULT,
        &m_depth_replacement_texture, nullptr
    ))) {
        printf("ActiveStream failed to create depth texture\n");
        return;
    }

    if (FAILED(m_depth_replacement_texture->GetSurfaceLevel(0, &m_depth_replacement_surface)))
    {
        printf("ActiveStream failed to get new depth surface\n");
        return;
    }

    if (color_desc.MultiSampleType == D3DMULTISAMPLE_NONE)
        m_color_replacement = m_color;
    else
    {
        if (FAILED(device->CreateRenderTarget(
            color_desc.Width, color_desc.Height,
            color_desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE,
            &m_color_replacement, nullptr
        ))) {
            printf("ActiveStream failed to create render target\n");
        }
    }
}