#pragma once
#include "Hooks.h"
#include <SDK/istudiorender.h>

DECL_EVENT(EVENT_DRAW_PROP);
DECL_EVENT(EVENT_PRE_DRAW_MODEL_EXECUTE);
DECL_EVENT(EVENT_POST_DRAW_MODEL_EXECUTE);

struct ModelRenderInfo_t;
struct StaticPropRenderInfo_t;
struct DrawModelState_t;
struct ModelRenderInfo_t;

typedef int(__thiscall* DrawModelExStaticPropFn_t)(void*, ModelRenderInfo_t& pInfo);
typedef int(__thiscall* DrawStaticPropArrayFastFn_t)(void*, StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth);
typedef void (__thiscall* DrawModelExecuteFn_t)(void*, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

class CModelRenderHook : public CBaseHook
{
public:
    struct Ctx;
    
    CModelRenderHook();

    void Hook() override;
    void Unhook() override;
    Ctx* Context() { return &m_ctx; }
    int DrawModelExStaticProp(ModelRenderInfo_t& pInfo);
    void DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

private:
    static int __stdcall Hooked_DrawModelExStaticProp(ModelRenderInfo_t& pInfo);
    static void __stdcall Hooked_DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

    struct Ctx
    {
        union
        {
            struct { ModelRenderInfo_t* pInfo; } static_prop;
            struct
            {
                const DrawModelState_t* state;
                const ModelRenderInfo_t* pInfo;
                matrix3x4_t* pCustomBoneToWorld;
            } model_execute;
        };
    };

    CVMTHook m_hook_model_render;
    Ctx m_ctx;
};

inline CModelRenderHook g_hk_model_render;