#include "world.h"
#include <Hooks/ModelRenderHook.h>
#include <Modules/Menu.h>

WorldModule g_world_module;

void WorldModule::StartListening()
{
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
    Listen(EVENT_DRAW_PROP, [this]{ return OnDrawProp(); });
    Listen(EVENT_DRAW_PROP_ARRAY, [this]{ return OnDrawPropArray(); });
}

int WorldModule::OnMenu() {
    if (ImGui::CollapsingHeader("World"))
    {
        ImGui::BeginGroup();
        ImGui::Checkbox("Draw props", &m_draw_props);
        ImGui::EndGroup();
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