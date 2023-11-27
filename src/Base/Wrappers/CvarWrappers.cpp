#include "CvarWrappers.h"
#include <SDK/InterfaceVersions/IEngineCvarSDK.h>
#include <SDK/InterfaceVersions/IEngineCvar007.h>

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