#include "Base.h"
#include <cassert>
#include <stdio.h>

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;
	CreateThread(0, 0, &Base::HookThread, 0, 0, 0);
}

DWORD __stdcall Base::HookThread(LPVOID Args)
{

	return 0;
}

HMODULE Base::GetModule(const char* Module)
{
	HMODULE hMod = GetModuleHandleA(Module);
	if (!hMod)
		FATAL("Failed GetModuleHandle to %s", Module);
	return hMod;
}

void Base::Fatal(const char* Title, const char* Format, ...)
{
	char buf[512];

	va_list va;
	va_start(va, Format);
	vsprintf_s(buf, Format, va);
	va_end(va);

	MessageBoxA(0, Title, buf, MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), -1);
}
