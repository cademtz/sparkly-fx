#pragma once
#include "Hooks.h"

typedef const char*(__thiscall *CDemoFile_ReadConsoleCommandFn)(void*);

class CEngineHook : public CBaseHook
{
public:
	CEngineHook() : BASEHOOK(CEngineHook) {}

	void Hook() override;
	void Unhook() override;
private:
	CJumpHook m_readconcmd_hook = {};

	static inline CDemoFile_ReadConsoleCommandFn m_orig_readconcmd = nullptr;

	static const char* __fastcall Hooked_CDemoFile_ReadConsoleCommand(void* _this);
};

inline CEngineHook g_hk_engine;