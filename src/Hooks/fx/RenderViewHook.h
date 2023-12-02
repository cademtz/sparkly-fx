#pragma once
#include <Hooks/Hooks.h>
#include <cstdint>

DECL_EVENT(EVENT_VIEW_DRAW_FADE);

class IMaterial;

class RenderViewHook : public CBaseHook
{
public:
    RenderViewHook();
    void Hook() override;
    void Unhook() override;
    void ViewDrawFade(uint8_t* color, IMaterial* pFadeMaterial);

private:
    CVMTHook m_hook;

    static void __stdcall Hooked_ViewDrawFade(UNCRAP uint8_t* color, IMaterial* pFadeMaterial);
};

inline RenderViewHook g_hk_renderview;