#pragma once
#include <Hooks/Hooks.h>

DECL_EVENT(EVENT_DRAWMODELSTATICPROP);

struct DrawModelInfo_t;
struct matrix3x4_t;

class StudioRenderHook : public CBaseHook
{
public:
    StudioRenderHook();
    void Hook() override;
    void Unhook() override;
    void DrawModelStaticProp(const DrawModelInfo_t& drawInfo, const matrix3x4_t& modelToWorld, int flags);

private:
    CVMTHook m_hook;

    static void __stdcall Hooked_DrawModelStaticProp(UNCRAP const DrawModelInfo_t& drawInfo, const matrix3x4_t& modelToWorld, int flags);
};

inline StudioRenderHook g_hk_studiorender;