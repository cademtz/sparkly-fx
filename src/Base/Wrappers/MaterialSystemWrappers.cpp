#include "MaterialSystemWrappers.h"
#include <SDK/imaterialsystem.h>
#include <SDK/InterfaceVersions/MaterialSystem081.h>

IMatRenderContext* IMaterialSystemWrapperSDK::GetRenderContext() {
    return m_int->GetRenderContext();
}
IMaterial* IMaterialSystemWrapperSDK::CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) {
    return m_int->CreateMaterial(pMaterialName, pVMTKeyValues);
}


IMatRenderContext* IMaterialSystemWrapper081::GetRenderContext() {
    return m_int->GetRenderContext();
}
IMaterial* IMaterialSystemWrapper081::CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) {
    return m_int->CreateMaterial(pMaterialName, pVMTKeyValues);
}