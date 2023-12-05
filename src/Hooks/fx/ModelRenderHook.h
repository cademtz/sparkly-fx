#pragma once
#include <Hooks/Hooks.h>
#include <SDK/istudiorender.h>
#include <unordered_set>
#include <string>
#include <Helper/threading.h>

DECL_EVENT(EVENT_PRE_DRAW_MODEL_EXECUTE);
DECL_EVENT(EVENT_POST_DRAW_MODEL_EXECUTE);

struct ModelRenderInfo_t;
struct DrawModelState_t;

typedef void (__thiscall* DrawModelExecuteFn_t)(void*, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

class CModelRenderHook : public CBaseHook
{
public:
    struct Ctx;
    using LockedModelNames = Helper::LockedRef<const std::unordered_set<std::string>>;
    
    CModelRenderHook();

    void Hook() override;
    void Unhook() override;
    Ctx* Context() { return &m_ctx; }
    void DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

    /// @brief Lock and return the list of models that have been drawn
    LockedModelNames GetDrawnModelList();
    /// @brief Clear the list of drawn models
    void ClearDrawnModelList();

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
    //std::unordered_set<const struct model_t*> m_drawn_models;
    std::unordered_set<std::string> m_drawn_modelnames;
    std::mutex m_drawn_models_mutex;
};

inline CModelRenderHook g_hk_model_render;