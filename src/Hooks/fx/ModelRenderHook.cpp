#include "ModelRenderHook.h"
#include <Base/Interfaces.h>

CModelRenderHook::CModelRenderHook() : BASEHOOK(CModelRenderHook)
{
    RegisterEvent(EVENT_PRE_DRAW_MODEL_EXECUTE);
    RegisterEvent(EVENT_POST_DRAW_MODEL_EXECUTE);
}

void CModelRenderHook::Hook()
{
    m_hook_model_render.Hook(Interfaces::model_render->Inst());
    m_hook_model_render.Set(Interfaces::model_render->GetOffset(Off_DrawModelExecute), &Hooked_DrawModelExecute);
}

void CModelRenderHook::Unhook()
{
    m_hook_model_render.Unhook();
}

void CModelRenderHook::DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
    static auto original = m_hook_model_render.Get<DrawModelExecuteFn_t>(Interfaces::model_render->GetOffset(Off_DrawModelExecute));
	original(Interfaces::model_render->Inst(), state, pInfo, pCustomBoneToWorld);
}

void __stdcall CModelRenderHook::Hooked_DrawModelExecute(UNCRAP const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
    auto* ctx = &g_hk_model_render.Context()->model_execute;
    ctx->state = &state;
    ctx->pInfo = &pInfo;
    ctx->pCustomBoneToWorld = pCustomBoneToWorld;

    int flags = g_hk_model_render.PushEvent(EVENT_PRE_DRAW_MODEL_EXECUTE);
    if (!(flags & Return_NoOriginal))
        g_hk_model_render.DrawModelExecute(*ctx->state, *ctx->pInfo, ctx->pCustomBoneToWorld);
    g_hk_model_render.PushEvent(EVENT_POST_DRAW_MODEL_EXECUTE);
}
