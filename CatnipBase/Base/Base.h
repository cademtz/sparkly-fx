#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "AsmTools.h"

#define FATAL(...) Base::Fatal("Fatal error in " __FUNCTION__, __VA_ARGS__)

namespace Base
{
	void OnAttach(HMODULE Module);
	DWORD WINAPI HookThread(LPVOID Args);

	// Use these if failure to get a handle is not an option
	HMODULE GetModule(const char* Module);
	FARPROC GetProc(HMODULE Module, const char* Proc);
	void Fatal(const char* Title, const char* Format, ...);

	inline HMODULE hInst;
	inline HWND hWnd;
};