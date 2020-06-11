#pragma once
#include <Windows.h>
#include "Hooks.h"

#define EVENT_WINDOWPROC "Event_WindowProc"

struct WindowProc_Ctx
{
	HWND hwnd;
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;
};

class CWindowHook : public CBaseHook
{
public:
	CWindowHook(HWND Window);
	void Unhook() override;

	inline HWND Window() const { return m_hwnd; }
	inline WNDPROC OldProc() const { return m_oldproc; }
	inline WindowProc_Ctx& Context() { return m_ctx; }

private:
	HWND m_hwnd;
	WNDPROC m_oldproc;
	WindowProc_Ctx m_ctx;

	static LRESULT WINAPI Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};