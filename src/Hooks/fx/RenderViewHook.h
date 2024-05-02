#pragma once
#include <Hooks/Hooks.h>
#include <cstdint>

class IMaterial;

/**
 * @brief Hook ViewDrawFade to control flashes from flashbangs and teleporters.
 */
class RenderViewHook : public CBaseHook
{
public:
    RenderViewHook();
    void Hook() override;
    void Unhook() override;
    void ViewDrawFade(uint8_t* color, IMaterial* pFadeMaterial);

    using ViewDrawFadeEvent_t = EventSource<void(uint8_t* color, IMaterial* pFadeMaterial)>;
    static inline ViewDrawFadeEvent_t OnViewDrawFade;

private:
    CVMTHook m_hook;

    static void __stdcall Hooked_ViewDrawFade(UNCRAP uint8_t* color, IMaterial* pFadeMaterial);
};

inline RenderViewHook g_hk_renderview;