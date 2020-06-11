#include "Hooks.h"
#include <Windows.h>

CBaseHook::~CBaseHook()
{
	Unhook();
	m_hooks.remove(this);
}

void CBaseHook::HookAll()
{
	for (auto hook : m_hooks)
		hook->Hook();
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
	}
}

void CVMTHook::Unhook()
{
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