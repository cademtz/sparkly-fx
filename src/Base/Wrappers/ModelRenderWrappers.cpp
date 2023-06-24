#include "ModelRenderWrappers.h"
#include <SDK/ivmodelrender.h>

int IVModelRenderWrapperSDK::GetOffset(EOffsets Offset)
{
    switch (Offset)
    {
    case Off_DrawModelExecute: return 19;
    case Off_DrawModelExStaticProp: return 17;
    case Off_DrawStaticPropArrayFast: return 21;
    default: return -1;
    }
}

void IVModelRenderWrapperSDK::ForcedMaterialOverride(IMaterial* newMaterial, OverrideType_t nOverrideType) {
    m_int->ForcedMaterialOverride(newMaterial, nOverrideType);
}
void IVModelRenderWrapperSDK::DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld) {
    m_int->DrawModelExecute(state, pInfo, pCustomBoneToWorld);
}