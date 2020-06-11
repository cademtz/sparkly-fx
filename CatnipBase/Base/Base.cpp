#include "Base.h"
#include "Hooks/WindowHook.h"
#include "Hooks/OverlayHook.h"
#include "Modules/Menu/Menu.h"
#include "Interfaces.h"

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;
	CreateThread(0, 0, &Base::HookThread, 0, 0, 0);
}

DWORD WINAPI Base::HookThread(LPVOID Args)
{
	while (!(hWnd = FindWindowA("Valve001", 0)))
		Sleep(100);

	Interfaces::CreateInterfaces();
	CBaseHook::HookAll();
	new CMenu;

	return 0;
}

HMODULE Base::GetModule(const char* Module)
{
	HMODULE hMod = GetModuleHandleA(Module);
	if (!hMod)
		FATAL("Failed GetModuleHandle to '%s'", Module);
	return hMod;
}

FARPROC Base::GetProc(HMODULE Module, const char* Proc)
{
	FARPROC result = GetProcAddress(Module, Proc);
	if (!result)
	{
		char name[MAX_PATH];
		GetModuleFileNameA(Module, name, MAX_PATH);
		FATAL("Failed GetProcAddress to '%s' in '%s'", Proc, name);
	}
	return result;
}

void Base::Fatal(const char* Title, const char* Format, ...)
{
	char buf[512];

	va_list va;
	va_start(va, Format);
	vsprintf_s(buf, Format, va);
	va_end(va);

	MessageBoxA(0, buf, Title, MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), -1);
}
