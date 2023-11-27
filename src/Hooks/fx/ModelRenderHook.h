#pragma once
#include <Hooks/Hooks.h>
#include <SDK/istudiorender.h>

DECL_EVENT(EVENT_PRE_DRAW_MODEL_EXECUTE);
DECL_EVENT(EVENT_POST_DRAW_MODEL_EXECUTE);

struct ModelRenderInfo_t;
struct DrawModelState_t;

typedef void (__thiscall* DrawModelExecuteFn_t)(void*, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

class CModelRenderHook : public CBaseHook
{
public:
    struct Ctx;
    
    CModelRenderHook();

    void Hook() override;
    void Unhook() override;
    Ctx* Context() { return &m_ctx; }
    void DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

private:
    static void __stdcall Hooked_DrawModelExecute(UNCRAP const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

    struct Ctx
    {
        struct
        {
            const DrawModelState_t* state;
            const ModelRenderInfo_t* pInfo;
            matrix3x4_t* pCustomBoneToWorld;
        } model_execute;
    };

    CVMTHook m_hook_model_render;
    Ctx m_ctx;
};

inline CModelRenderHook g_hk_model_render;