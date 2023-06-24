#include "ModelRenderHook.h"
#include <Base/Interfaces.h>

CModelRenderHook::CModelRenderHook() : BASEHOOK(CModelRenderHook)
{
    RegisterEvent(EVENT_DRAW_PROP);
    RegisterEvent(EVENT_DRAW_PROP_ARRAY);
    RegisterEvent(EVENT_PRE_DRAW_MODEL_EXECUTE);
    RegisterEvent(EVENT_POST_DRAW_MODEL_EXECUTE);
}

void CModelRenderHook::Hook()
{
    m_hook_model_render.Hook(Interfaces::model_render->Inst());
    m_hook_model_render.Set(Interfaces::model_render->GetOffset(Off_DrawModelExStaticProp), &Hooked_DrawModelExStaticProp);
    m_hook_model_render.Set(Interfaces::model_render->GetOffset(Off_DrawStaticPropArrayFast), &Hooked_DrawStaticPropArrayFast);
    m_hook_model_render.Set(Interfaces::model_render->GetOffset(Off_DrawModelExecute), &Hooked_DrawModelExecute);
}

void CModelRenderHook::Unhook()
{
    m_hook_model_render.Unhook();
}

int CModelRenderHook::DrawModelExStaticProp(ModelRenderInfo_t& pInfo)
{
    static auto original = m_hook_model_render.Get<DrawModelExStaticPropFn_t>(Interfaces::model_render->GetOffset(Off_DrawModelExStaticProp));
	return original(Interfaces::model_render->Inst(), pInfo);
}

int CModelRenderHook::DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth)
{
    static auto original = m_hook_model_render.Get<DrawStaticPropArrayFastFn_t>(Interfaces::model_render->GetOffset(Off_DrawStaticPropArrayFast));
	return original(Interfaces::model_render->Inst(), pProps, count, bShadowDepth);
}

void CModelRenderHook::DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
    static auto original = m_hook_model_render.Get<DrawModelExecuteFn_t>(Interfaces::model_render->GetOffset(Off_DrawModelExecute));
	original(Interfaces::model_render->Inst(), state, pInfo, pCustomBoneToWorld);
}

int __stdcall CModelRenderHook::Hooked_DrawModelExStaticProp(ModelRenderInfo_t& pInfo)
{
    auto* ctx = &g_hk_model_render.Context()->static_prop;
    ctx->pInfo = &pInfo;

    int flags = g_hk_model_render.PushEvent(EVENT_DRAW_PROP);
    if (flags & Return_NoOriginal)
        return 1;
    return g_hk_model_render.DrawModelExStaticProp(*ctx->pInfo);
}

int __stdcall CModelRenderHook::Hooked_DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth)
{
    auto* ctx = &g_hk_model_render.Context()->static_prop_array;
    ctx->pProps = pProps;
    ctx->count = count;
    ctx->bShadowDepth = bShadowDepth;

    int flags = g_hk_model_render.PushEvent(EVENT_DRAW_PROP_ARRAY);
    if (flags & Return_NoOriginal)
        return 1;
    return g_hk_model_render.DrawStaticPropArrayFast(ctx->pProps, ctx->count, ctx->bShadowDepth);
}

void __stdcall CModelRenderHook::Hooked_DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
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
