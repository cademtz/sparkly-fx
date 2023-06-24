#pragma once
#include "Wrappers.h"

class IVModelRender;

class IVModelRenderWrapperSDK : public IVModelRenderWrapper
{
    IVModelRender* m_int;

public:
    IVModelRenderWrapperSDK(void* instance) : m_int((IVModelRender*)instance) {}
    void* Inst() override { return m_int; }
    int GetOffset(EOffsets Offset) override;

    void ForcedMaterialOverride(IMaterial* newMaterial, OverrideType_t nOverrideType) override;
    void DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld) override;
};
