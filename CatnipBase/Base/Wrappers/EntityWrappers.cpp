#include "EntityWrappers.h"
#include "SDK/InterfaceVersions/EntitySDK.h"
#include "SDK/InterfaceVersions/EntityCSGO.h"

bool CEntityWrapperSDK::SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) {
    return m_ent->SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}
ClientClass* CEntityWrapperSDK::GetClientClass() {
    return m_ent->GetClientClass();
}
bool CEntityWrapperSDK::IsDormant(void) {
    return m_ent->IsDormant();
}
int CEntityWrapperSDK::entindex(void) const {
    return m_ent->entindex();
}
