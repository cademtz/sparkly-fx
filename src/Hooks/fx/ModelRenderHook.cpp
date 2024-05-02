#include "ModelRenderHook.h"
#include <Base/Interfaces.h>
#include <SDK/ivmodelrender.h>

CModelRenderHook::CModelRenderHook() : BASEHOOK(CModelRenderHook) {}

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

CModelRenderHook::LockedModelNames CModelRenderHook::GetDrawnModelList() {
    return LockedModelNames(m_drawn_modelnames, m_drawn_models_mutex);
}

void CModelRenderHook::ClearDrawnModelList()
{
    std::lock_guard lock(m_drawn_models_mutex);
    m_drawn_modelnames.clear();
}

void __stdcall CModelRenderHook::Hooked_DrawModelExecute(UNCRAP const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
    {
        std::lock_guard lock(g_hk_model_render.m_drawn_models_mutex);
        g_hk_model_render.m_drawn_modelnames.emplace(state.m_pStudioHdr->name);
    }

    int flags = OnPreDrawModelExecute.DispatchEvent(state, pInfo, pCustomBoneToWorld);
    if (!(flags & EventReturnFlags::NoOriginal))
        g_hk_model_render.DrawModelExecute(state, pInfo, pCustomBoneToWorld);
    OnPostDrawModelExecute.DispatchEvent(state, pInfo, pCustomBoneToWorld);
}
