#include "ActiveStream.h"
#include <Hooks/ModelRenderHook.h>
#include <Hooks/ClientHook.h>
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <SDK/ivrenderview.h>
#include <SDK/client_class.h>
#include <SDK/KeyValues.h>
#include <SDK/cdll_int.h>
#include <SDK/view_shared.h>

void ActiveStream::StartListening()
{
    m_matte_material = CreateMatteMaterial();
    if (!m_matte_material)
        printf(__FUNCTION__ ": Couldn't create matte material");
    
    Listen(EVENT_DRAW_PROP, [this]{ return OnDrawProp(); });
    Listen(EVENT_DRAW_PROP_ARRAY, [this] { return OnDrawPropArray(); });
    Listen(EVENT_PRE_DRAW_MODEL_EXECUTE, [this] { return PreDrawModelExecute(); });
    Listen(EVENT_POST_DRAW_MODEL_EXECUTE, [this] { return PostDrawModelExecute(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this] { return OnFrameStageNotify(); });
    Listen(EVENT_OVERRIDEVIEW, [this] { return OnOverrideView(); });
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
}

void ActiveStream::SignalUpdate(Stream::Ptr stream)
{
    auto lock = WriteLock();
    if (m_stream == nullptr)
        return;
    m_should_update_materials = stream == nullptr || m_stream == stream;
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

int ActiveStream::OnDrawProp()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;

    auto tweak = m_stream->begin<PropRenderTweak>();
    if (tweak == m_stream->end<PropRenderTweak>())
        return 0;
    
    if (tweak->hide)
        return Return_NoOriginal;
    return 0;
}

int ActiveStream::OnDrawPropArray() {
    return OnDrawProp();
}

int ActiveStream::PreDrawModelExecute()
{
    auto lock = ReadLock();
    if (!m_stream)
        return 0;
    
    auto* ctx = &g_hk_model_render.Context()->model_execute;

    CBaseEntity* entity = Interfaces::entlist->GetClientEntity(ctx->pInfo->entity_index);
    if (entity == nullptr || entity->GetClientClass() == nullptr)
        return 0;

    for (auto tweak = m_stream->begin<EntityFilterTweak>(); tweak != m_stream->end<EntityFilterTweak>(); ++tweak)
    {
        if (!tweak->IsEntityAffected(entity->GetClientClass()->GetName()))
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

        // TODO: If tweak->render_effect == RenderEffect::MATTE, apply a special matte material
        if (tweak->render_effect == EntityFilterTweak::RenderEffect::MATTE)
            Interfaces::model_render->ForcedMaterialOverride(m_matte_material);
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
    if (stage == FRAME_RENDER_START)
    {
        auto lock = ReadLock();
        if (m_should_update_materials)
            UpdateMaterials();
    }
    return 0;
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

IMaterial* ActiveStream::CreateMatteMaterial()
{
    KeyValues* vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    vmt_values->SetBool("$nocull", false);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$halflambert", true);
    vmt_values->SetBool("$nofog", true);
    vmt_values->SetBool("$znearer", true);
    
    // The KeyValues instance is owned by the new material, and should not be deleted
    IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_matte.vmt", vmt_values);
    if (!mat)
        vmt_values->deleteThis();
    return mat;
}
