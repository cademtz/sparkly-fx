#include "MaterialSystemWrappers.h"
#include <SDK/imaterialsystem.h>
#include <SDK/InterfaceVersions/MaterialSystem081.h>
#include <SDK/InterfaceVersions/MaterialSystemGMod.h>

IMatRenderContext* IMaterialSystemWrapperSDK::GetRenderContext() {
    return m_int->GetRenderContext();
}
IMaterial* IMaterialSystemWrapperSDK::CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) {
    return m_int->CreateMaterial(pMaterialName, pVMTKeyValues);
}
MaterialHandle_t IMaterialSystemWrapperSDK::FirstMaterial() {
    return m_int->FirstMaterial();
}
MaterialHandle_t IMaterialSystemWrapperSDK::NextMaterial(MaterialHandle_t handle) {
    return m_int->NextMaterial(handle );
}
MaterialHandle_t IMaterialSystemWrapperSDK::InvalidMaterial() {
    return m_int->InvalidMaterial();
}
IMaterial* IMaterialSystemWrapperSDK::GetMaterial(MaterialHandle_t handle) {
    return m_int->GetMaterial(handle);
}
ITexture* IMaterialSystemWrapperSDK::FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) {
    return m_int->FindTexture(pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags);
}

IMatRenderContext* IMaterialSystemWrapper081::GetRenderContext() {
    return m_int->GetRenderContext();
}
IMaterial* IMaterialSystemWrapper081::CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) {
    return m_int->CreateMaterial(pMaterialName, pVMTKeyValues);
}
MaterialHandle_t IMaterialSystemWrapper081::FirstMaterial() {
    return m_int->FirstMaterial();
}
MaterialHandle_t IMaterialSystemWrapper081::NextMaterial(MaterialHandle_t handle) {
    return m_int->NextMaterial(handle );
}
MaterialHandle_t IMaterialSystemWrapper081::InvalidMaterial() {
    return m_int->InvalidMaterial();
}
IMaterial* IMaterialSystemWrapper081::GetMaterial(MaterialHandle_t handle) {
    return m_int->GetMaterial(handle);
}
ITexture* IMaterialSystemWrapper081::FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) {
    return m_int->FindTexture(pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags);
}

IMatRenderContext* IMaterialSystemWrapperGMod::GetRenderContext() {
    return m_int->GetRenderContext();
}
IMaterial* IMaterialSystemWrapperGMod::CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) {
    return m_int->CreateMaterial(pMaterialName, pVMTKeyValues);
}
MaterialHandle_t IMaterialSystemWrapperGMod::FirstMaterial() {
    return m_int->FirstMaterial();
}
MaterialHandle_t IMaterialSystemWrapperGMod::NextMaterial(MaterialHandle_t handle) {
    return m_int->NextMaterial(handle );
}
MaterialHandle_t IMaterialSystemWrapperGMod::InvalidMaterial() {
    return m_int->InvalidMaterial();
}
IMaterial* IMaterialSystemWrapperGMod::GetMaterial(MaterialHandle_t handle) {
    return m_int->GetMaterial(handle);
}
ITexture* IMaterialSystemWrapperGMod::FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) {
    return m_int->FindTexture(pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags);
}

