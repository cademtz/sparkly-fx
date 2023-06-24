#include "ActiveRenderConfig.h"
#include <Hooks/ModelRenderHook.h>
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <SDK/ivrenderview.h>
#include <SDK/client_class.h>

void ActiveRenderConfig::StartListening()
{
    Listen(EVENT_DRAW_PROP, [this]{ return OnDrawProp(); });
    Listen(EVENT_DRAW_PROP_ARRAY, [this] { return OnDrawPropArray(); });
    Listen(EVENT_PRE_DRAW_MODEL_EXECUTE, [this] { return PreDrawModelExecute(); });
    Listen(EVENT_POST_DRAW_MODEL_EXECUTE, [this] { return PostDrawModelExecute(); });
}

void ActiveRenderConfig::Set(RenderConfig::Ptr config) {
    m_config = config;
}

void ActiveRenderConfig::SetNone() {
    m_config = nullptr;
}

bool ActiveRenderConfig::ArePropsVisible() const
{
    if (m_config)
    {
        for (auto it = m_config->begin<PropRenderTweak>(); it != m_config->end<PropRenderTweak>(); ++it)
        {
            if (it->hide)
                return false;
        }
    }
    return true;
}

int ActiveRenderConfig::OnDrawProp() {
    return ArePropsVisible() ? 0 : Return_NoOriginal;
}

int ActiveRenderConfig::OnDrawPropArray() {
    return ArePropsVisible() ? 0 : Return_NoOriginal;
}

int ActiveRenderConfig::PreDrawModelExecute()
{
    if (!m_config)
        return 0;
    
    auto* ctx = &g_hk_model_render.Context()->model_execute;

    CBaseEntity* entity = Interfaces::entlist->GetClientEntity(ctx->pInfo->entity_index);
    if (entity == nullptr || entity->GetClientClass() == nullptr)
        return 0;

    for (auto tweak = m_config->begin<EntityFilterTweak>(); tweak != m_config->end<EntityFilterTweak>(); ++tweak)
    {
        if (!tweak->IsEntityAffected(entity->GetClientClass()->GetName()))
            continue;
        
        if (tweak->IsEffectInvisible())
            return Return_NoOriginal;

        // Store original draw parameters
        m_is_dme_affected = true;
        Interfaces::render_view->GetColorModulation(m_last_dme_params.color.data());
        m_last_dme_params.color[3] = Interfaces::render_view->GetBlend();
        
        Interfaces::render_view->SetColorModulation(tweak->color_multiply.data());
        Interfaces::render_view->SetBlend(tweak->color_multiply[3]);

        // TODO: If tweak->render_effect == RenderEffect::MATTE, apply a special matte material
    }
    return 0;
}

int ActiveRenderConfig::PostDrawModelExecute()
{
    if (m_is_dme_affected)
    {
        Interfaces::render_view->SetColorModulation(m_last_dme_params.color.data());
        Interfaces::render_view->SetBlend(m_last_dme_params.color[3]);
    }

    m_is_dme_affected = false;
    return 0;
}