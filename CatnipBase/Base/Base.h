#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "AsmTools.h"

#define FATAL(...) Base::Fatal("Fatal error in " __FUNCTION__, __VA_ARGS__)

namespace Base
{
	void OnAttach(HMODULE Module);
	DWORD WINAPI HookThread(LPVOID Args);

	HMODULE GetModule(const char* Module);
	void Fatal(const char* Title, const char* Format, ...);

	inline HMODULE hInst;
	inline HWND hWnd;
};