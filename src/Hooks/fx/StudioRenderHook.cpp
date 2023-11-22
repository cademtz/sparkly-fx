#include "StudioRenderHook.h"
#include <Base/Interfaces.h>
#include <SDK/istudiorender.h>

#define INDEX_DRAWMODELSTATICPROP 30

StudioRenderHook::StudioRenderHook() : BASEHOOK(StudioRenderHook) {
    RegisterEvent(EVENT_DRAWMODELSTATICPROP);
}

void StudioRenderHook::Hook()
{
    m_hook.Hook(Interfaces::studio_render);
    m_hook.Set(INDEX_DRAWMODELSTATICPROP, &Hooked_DrawModelStaticProp);
}

void StudioRenderHook::Unhook() {
    m_hook.Unhook();
}

void StudioRenderHook::DrawModelStaticProp(const DrawModelInfo_t& drawInfo, const matrix3x4_t& modelToWorld, int flags)
{
    typedef void(__thiscall* prototype)(void*, const DrawModelInfo_t&, const matrix3x4_t& modelToWorld, int flags);
    static auto original = m_hook.Get<prototype>(INDEX_DRAWMODELSTATICPROP);
    original(Interfaces::studio_render, drawInfo, modelToWorld, flags);
}

void __stdcall StudioRenderHook::Hooked_DrawModelStaticProp(UNCRAP const DrawModelInfo_t& drawInfo, const matrix3x4_t& modelToWorld, int flags)
{
    int flags = g_hk_studiorender.PushEvent(EVENT_DRAWMODELSTATICPROP);
    if (flags & Return_NoOriginal)
        return;
    g_hk_studiorender.DrawModelStaticProp(drawInfo, modelToWorld, flags);
}
