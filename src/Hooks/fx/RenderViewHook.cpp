#include "RenderViewHook.h"
#include <Base/Interfaces.h>

#define INDEX_VIEWDRAWFADE 24

RenderViewHook::RenderViewHook() : BASEHOOK(RenderViewHook) {
    RegisterEvent(EVENT_VIEW_DRAW_FADE);
}

void RenderViewHook::Hook()
{
    m_hook.Hook(Interfaces::render_view);
    m_hook.Set(INDEX_VIEWDRAWFADE, &Hooked_ViewDrawFade);
}

void RenderViewHook::Unhook() {
    m_hook.Unhook();
}

void RenderViewHook::ViewDrawFade(uint8_t* color, IMaterial* pFadeMaterial)
{
    typedef void(__thiscall* prototype)(void*, uint8_t*, IMaterial*);
    static auto original = m_hook.Get<prototype>(INDEX_VIEWDRAWFADE);
    original(Interfaces::render_view, color, pFadeMaterial);
}

void __stdcall RenderViewHook::Hooked_ViewDrawFade(UNCRAP uint8_t* color, IMaterial* pFadeMaterial)
{
    int event_flags = g_hk_renderview.PushEvent(EVENT_VIEW_DRAW_FADE);
    if (event_flags & Return_NoOriginal)
        return;
    g_hk_renderview.ViewDrawFade(color, pFadeMaterial);
}
