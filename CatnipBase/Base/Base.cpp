#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Base.h"

HMODULE GetModuleHandleSafe(const char* Module)
{
	HMODULE HMod = nullptr;

	while (!HMod)
	{
		HMod = GetModuleHandleA(Module);
		Sleep(1);
	}

	return HMod;
}

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;

	//client_dll = GetModuleHandleSafe("client.dll");
}
