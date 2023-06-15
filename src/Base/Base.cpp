#include "Base.h"
#include <Hooks/WindowHook.h>
#include <Hooks/OverlayHook.h>
#include <Modules/BaseModule.h>
#include <SDK/mathlib.h>
#include "Interfaces.h"
#include "Netvars.h"

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;
	CreateThread(0, 0, &Base::HookThread, 0, 0, 0);
}

DWORD WINAPI Base::HookThread(LPVOID Args)
{
	while (!(hWnd = FindWindowA("Valve001", 0)))
		Sleep(100);

	MathLib_Init();

#ifdef _DEBUG
	AllocConsole();
	SetConsoleTitleA("catnitch baes");
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
#endif

	Interfaces::CreateInterfaces();
	Netvars::GetNetvars();
	CBaseHook::HookAll();
	CModule::StartAll();

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
