#include "CvarWrappers.h"
#include <SDK/InterfaceVersions/IEngineCvarSDK.h>
#include <SDK/InterfaceVersions/IEngineCvar007.h>

int ICvarWrapperSDK::AllocateDLLIdentifier() {
    return m_int->AllocateDLLIdentifier();
}
int ICvarWrapperSDK::ProcessQueuedMaterialThreadConVarSets() {
    return m_int->ProcessQueuedMaterialThreadConVarSets();
}
void ICvarWrapperSDK::UnregisterConCommands(int i) {
    m_int->UnregisterConCommands(i);
}
void ICvarWrapperSDK::RegisterConCommand(ConCommandBase* pCommandBase) {
    m_int->RegisterConCommand(pCommandBase);
}
void ICvarWrapperSDK::UnregisterConCommand(ConCommandBase* pCommandBase) {
    m_int->UnregisterConCommand(pCommandBase);
}
ConVar* ICvarWrapperSDK::FindVar(const char *var_name) {
    return m_int->FindVar(var_name);
}
ConCommand*	ICvarWrapperSDK::FindCommand(const char *name) {
    return m_int->FindCommand(name);
}
ConCommandBase* ICvarWrapperSDK::GetCommands() {
    return m_int->GetCommands();
}

int ICvarWrapper007::AllocateDLLIdentifier() {
    return m_int->AllocateDLLIdentifier();
}
int ICvarWrapper007::ProcessQueuedMaterialThreadConVarSets() {
    return m_int->ProcessQueuedMaterialThreadConVarSets();
}
void ICvarWrapper007::UnregisterConCommands(int i) {
    m_int->UnregisterConCommands(i);
}
void ICvarWrapper007::RegisterConCommand(ConCommandBase* pCommandBase) {
    m_int->RegisterConCommand(pCommandBase);
}
void ICvarWrapper007::UnregisterConCommand(ConCommandBase* pCommandBase) {
    m_int->UnregisterConCommand(pCommandBase);
}
ConVar* ICvarWrapper007::FindVar(const char *var_name) {
    return m_int->FindVar(var_name);
}
ConCommand*	ICvarWrapper007::FindCommand(const char *name) {
    return m_int->FindCommand(name);
}
ConCommandBase* ICvarWrapper007::GetCommands() {
    return m_int->GetCommands();
}