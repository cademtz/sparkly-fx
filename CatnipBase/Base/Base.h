#pragma once
#include "BuildConfig.h"
#include "AsmTools.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define FATAL(...) Base::Fatal("Fatal error in " __FUNCTION__, __VA_ARGS__)

enum EAppID
{
	AppId_HL2 = 220,
	AppId_CSS = 240,
	AppID_HL2DM = 320,
	AppId_Portal = 400,
	AppId_TF2 = 440,
	AppId_L4D = 500,
	AppId_L4D2 = 550,
	AppId_Portal2 = 620,
	AppId_CSGO = 730,
	AppID_GMod = 4000,
};

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

#ifdef _WIN64
	constexpr bool Win64 = true;
#else
	constexpr bool Win64 = false;
#endif
};