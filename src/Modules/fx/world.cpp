#include "world.h"
#include <SDK/istudiorender.h>
#include <SDK/cdll_int.h>
#include <SDK/ivmodelrender.h>
#include <Hooks/ModelRenderHook.h>
#include <Hooks/ClientHook.h>
#include <Modules/Menu.h>

WorldModule g_world_module;

void WorldModule::StartListening()
{
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_DRAW_PROP, [this]{ return OnDrawProp(); });
    Listen(EVENT_DRAW_PROP_ARRAY, [this]{ return OnDrawPropArray(); });
    Listen(EVENT_FRAMESTAGENOTIFY, [this]{ return OnFrameStageNotify(); });
}

int WorldModule::OnMenu() {
    if (ImGui::CollapsingHeader("World"))
    {
        ImGui::BeginGroup();
        ImGui::Checkbox("Draw props", &m_draw_props);
        ImGui::ColorEdit3("Color modulation", m_colormod.data());
        ImGui::EndGroup();
    }
    return 0;
}

int WorldModule::OnFrameStageNotify()
{
    if (g_hk_client.Context()->curStage == FRAME_RENDER_START)
    {
        IVModelRender* model_render = (IVModelRender*)Interfaces::model_render->Inst();
        model_render->SetColorModulation(m_colormod.data());
    }
    return 0;
}

int WorldModule::OnDrawPropArray()
{
    if (!m_draw_props)
        return Return_NoOriginal;
    return 0;
}

int WorldModule::OnDrawProp()
{
    if (!m_draw_props)
        return Return_NoOriginal;
    return 0;
}