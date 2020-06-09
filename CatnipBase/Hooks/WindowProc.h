#pragma once
#include <Windows.h>
#include "Hooks.h"

class CWindowHook : public CBaseHook
{
public:
	CWindowHook(HWND Window);
	void Unhook() override;

	inline HWND Window() const { return m_hwnd; }
	inline WNDPROC OldProc() const { return m_oldproc; }

private:
	HWND m_hwnd;
	WNDPROC m_oldproc;

	static LRESULT WINAPI Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};