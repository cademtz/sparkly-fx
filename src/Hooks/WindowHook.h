#pragma once
#include <Windows.h>
#include "Hooks.h"

class CWindowHook : public CBaseHook
{
public:
	CWindowHook() : BASEHOOK(CWindowHook), m_hwnd(nullptr), m_oldproc(nullptr) {}

	void Hook() override;
	void Unhook() override;

	HWND Window() const { return m_hwnd; }
	WNDPROC OldProc() const { return m_oldproc; }

	BOOL SetCurPos(int X, int Y);
	int ShowCur(BOOL bShow);
	HCURSOR SetCur(HCURSOR hCursor);
	BOOL GetCurPos(LPPOINT Point);
	BOOL GetCurInfo(PCURSORINFO pci);

	void SetInputEnabled(bool Enabled);
	bool GetInputEnabled() const { return !m_passInput; }

	using WndProcEvent = EventSource<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
	static inline WndProcEvent OnWndProc;
private:
	CJumpHook m_hkcurpos, m_hkshowcur, m_hksetcur, m_hkgetcurpos, m_hk_getcurinfo;

	HWND m_hwnd;
	WNDPROC m_oldproc;
	bool m_passInput = true;

	struct
	{
		CURSORINFO cursor_info;
		int show_count;
	} m_lastInput = { 0 };

	static LRESULT WINAPI Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL WINAPI Hooked_SetCursorPos(int X, int Y);
	static int WINAPI Hooked_ShowCursor(BOOL bShow);
	static HCURSOR WINAPI Hooked_SetCursor(HCURSOR hCursor);
	static BOOL WINAPI Hooked_GetCursorPos(LPPOINT Point);
	static BOOL WINAPI Hooked_GetCursorInfo(PCURSORINFO pci);
};

inline CWindowHook g_hk_window;
