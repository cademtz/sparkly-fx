#pragma once
#include "Wrappers.h"

class ICvar;
class IEngineCvar007;
class ConCommand;
class ConCommandBase;

class ICvarWrapperSDK : public ICvarWrapper
{
    ICvar* m_int;

public:
    ICvarWrapperSDK(void* icvar) : m_int((ICvar*)icvar) {}
    
    void* Inst() override { return m_int; }

    void RegisterConCommand(ConCommandBase* pCommandBase) override;
    void UnregisterConCommand(ConCommandBase* pCommandBase) override;
    ConVar* FindVar (const char *var_name) override;
    ConCommand*	FindCommand(const char *name) override;
    ConCommandBase* GetCommands() override;
};

class ICvarWrapper007 : public ICvarWrapper
{
    IEngineCvar007* m_int;

public:
    ICvarWrapper007(void* icvar) : m_int((IEngineCvar007*)icvar) {}
    
    void* Inst() override { return m_int; }

    void RegisterConCommand(ConCommandBase* pCommandBase) override;
    void UnregisterConCommand(ConCommandBase* pCommandBase) override;
    ConVar* FindVar (const char *var_name) override;
    ConCommand*	FindCommand(const char *name) override;
    ConCommandBase* GetCommands() override;
};