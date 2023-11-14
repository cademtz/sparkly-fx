#include "Hooks.h"
#include <Windows.h>
#include <MinHook.h>
#include <Base/Sig.h>
#include <Base/AsmTools.h>

CBaseHook::~CBaseHook()
{
	Unhook();
	m_hooks.erase(m_hash);
}

void CBaseHook::HookAll()
{
	for (auto hook : m_hooks)
		hook.second->Hook();
}

void CBaseHook::UnHookAll()
{
	for (auto hook : m_hooks)
		hook.second->Unhook();
}

void CVMTHook::Hook(void* Instance, bool AllInstances)
{
	m_inst = Instance;
	m_all = AllInstances;
	void** vmt = *(void***)m_inst;
	for (m_count = 0; vmt[m_count]; m_count++);

	void** copy = new void* [m_count];
	memcpy(copy, vmt, sizeof(vmt[0]) * m_count);

	if (m_all)
	{
		m_oldvmt = copy;
		m_newvmt = vmt;
	}
	else
	{
		m_oldvmt = vmt;
		m_newvmt = copy;
		*(void***)m_inst = m_newvmt;
	}
}

void CVMTHook::Unhook()
{
	if (!IsHooked())
		return;

	size_t size = sizeof(m_oldvmt[0]) * m_count;

	if (m_all)
	{
		DWORD oldflags;
		VirtualProtect(m_newvmt, size, PAGE_EXECUTE_READWRITE, &oldflags);
		memcpy(m_newvmt, m_oldvmt, size);
		VirtualProtect(m_newvmt, size, oldflags, &oldflags);
		delete[] m_oldvmt;
	}
	else
	{
		*(void***)m_inst = m_oldvmt;
		delete[] m_newvmt;
	}

	m_inst = 0;
	m_oldvmt = m_newvmt = 0;
}

void CVMTHook::Set(size_t Index, void* Function)
{
	if (m_all)
	{
		size_t size = sizeof(m_newvmt[Index]);
		DWORD oldflags;
		VirtualProtect(&m_newvmt[Index], size, PAGE_EXECUTE_READWRITE, &oldflags);
		m_newvmt[Index] = Function;
		VirtualProtect(&m_newvmt[Index], size, oldflags, &oldflags);
	}
	else
		m_newvmt[Index] = Function;
}

void CJumpHook::Hook(void* From, void* To)
{
	MH_STATUS mh_status = MH_Initialize();
	if (mh_status != MH_OK && mh_status != MH_ERROR_ALREADY_INITIALIZED)
		FATAL("Failed to initialize minhook");

	if (MH_CreateHook(From, To, &m_original) != MH_OK)
		FATAL("Failed to create hook");

	if (MH_EnableHook(From) != MH_OK)
		FATAL("Failed to enable hook");

	m_from = From, m_to = To;
}

void CJumpHook::UnHook()
{
	if (m_from != nullptr)
	{
		MH_STATUS mh_status = MH_RemoveHook(m_from);
		if (mh_status != MH_OK)
			FATAL("Failed to destroy hook: %s", MH_StatusToString(mh_status));
	}

	m_from = nullptr, m_to = nullptr, m_original = nullptr;
}
