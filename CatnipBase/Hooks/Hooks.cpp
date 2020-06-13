#include "Hooks.h"
#include <Windows.h>

#define Zydis_EXPORTS // Some hacky crap because lazey
#define ZYDIS_DISABLE_FORMATTER
#include <Zydis/Zydis.h>

#pragma comment(lib, "Zydis.lib")

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

void CJumpHook::Hook(void* From, void* To, size_t Length)
{
	if (!Length)
	{
		ZydisDecoder de;
#ifdef _WIN64
		ZydisDecoderInit(&de, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
#define JMPFUNC AbsJmp
#define JMPSIZE 14
#else
		ZydisDecoderInit(&de, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);
#define JMPFUNC RelJmp
#define JMPSIZE 5
#endif
		ZyanUSize offset = 0;
		const ZyanUSize length = 0xFF;
		ZydisDecodedInstruction instruction;
		while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(
			&de, (char*)From + offset, length - offset, &instruction)))
		{
			offset += instruction.length;
			if (offset >= JMPSIZE)
			{
				Length = offset;
				break;
			}
		}
	}
	DWORD dwOld;
	VirtualProtect((void*)From, Length, PAGE_EXECUTE_READWRITE, &dwOld);

	m_original = (PBYTE)VirtualAlloc(nullptr, Length + JMPSIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(m_original, (void*)From, Length);

	JMPFUNC((UINT_PTR)m_original + Length, (UINT_PTR)From + Length);
	JMPFUNC((UINT_PTR)From, (UINT_PTR)To);

	VirtualProtect((void*)From, Length, dwOld, &dwOld);
	FlushInstructionCache(GetCurrentProcess(), 0, 0);

	m_loc = (void*)From, m_hooklen = Length;
	m_hooked = true;
}

void CJumpHook::Hook(const char* Module, const char* Function, void* To, size_t Length)
{
	HMODULE hMod = Base::GetModule(Module);
	FARPROC pFunc = Base::GetProc(hMod, Function);
	Hook(pFunc, To, Length);
}

void CJumpHook::UnHook()
{
	if (!m_hooked)
		return;

	DWORD dwOld;
	VirtualProtect(m_loc, m_hooklen, PAGE_EXECUTE_READWRITE, &dwOld);
	memcpy_s(m_loc, m_hooklen, m_original, m_hooklen);
	VirtualProtect(m_loc, m_hooklen, dwOld, &dwOld);
	VirtualFree(m_original, 0, MEM_RELEASE);
	FlushInstructionCache(GetCurrentProcess(), 0, 0);

	m_loc = nullptr, m_hooklen = 0;
	m_hooked = false;
}

void CJumpHook::RelJmp(UINT_PTR From, UINT_PTR To)
{
	*(BYTE*)From = 0xE9;
	*(DWORD*)(From + 1) = To - From - 5;
}

void CJumpHook::AbsJmp(UINT_PTR From, UINT_PTR To)
{
	PBYTE b = (PBYTE)From;

	*(WORD*)From = MAKEWORD(0xFF, 0x25);	// jmp [rip+imm32]
	*(DWORD*)&b[2] = 0;						// rip + 0
	*(UINT_PTR*)&b[6] = To;
}