#pragma once
#include "Hooks.h"

DECL_EVENT(EVENT_DRAW_MODEL);
DECL_EVENT(EVENT_DRAW_PROP);
DECL_EVENT(EVENT_DRAW_PROP_ARRAY);

struct ModelRenderInfo_t;
struct StaticPropRenderInfo_t;
typedef int(__thiscall* DrawModelExStaticPropFn_t)(void*, ModelRenderInfo_t& pInfo);
typedef int(__thiscall* DrawStaticPropArrayFastFn_t)(void*, StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth);

class CModelRenderHook : public CBaseHook
{
public:
    struct Ctx;
    
    CModelRenderHook();

    void Hook() override;
    void Unhook() override;
    Ctx* Context() { return &m_ctx; }
    int DrawModelExStaticProp(ModelRenderInfo_t& pInfo);
    int DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth);

private:
    static int __stdcall Hooked_DrawModelExStaticProp(ModelRenderInfo_t& pInfo);
    static int __stdcall Hooked_DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth);

    struct Ctx
    {
        union
        {
            struct { ModelRenderInfo_t* pInfo; } static_prop;
            struct
            {
                StaticPropRenderInfo_t* pProps;
                int count;
                bool bShadowDepth;
            } static_prop_array;
        };
    };

    CVMTHook m_hook_model_render;
    Ctx m_ctx;
};

inline CModelRenderHook g_hk_model_render;