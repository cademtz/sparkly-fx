#pragma once
#include <Windows.h>
#include "Hooks.h"

DECL_EVENT(EVENT_WINDOWPROC);
DECL_EVENT(EVENT_SETCURSORPOS);
DECL_EVENT(EVENT_SHOWCURSOR);
DECL_EVENT(EVENT_SETCURSOR);

struct WindowProc_Ctx
{
	HWND hwnd;
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;

	int cur_x, cur_y;
	HCURSOR hCursor, hPrevCursor;
};

class CWindowHook : public CBaseHook
{
public:
	CWindowHook();

	void Hook() override;
	void Unhook() override;

	inline HWND Window() const { return m_hwnd; }
	inline WNDPROC OldProc() const { return m_oldproc; }
	inline WindowProc_Ctx* Context() { return &m_ctx; }

	BOOL SetCurPos(int X, int Y);
	int ShowCur(BOOL bShow);
	HCURSOR SetCur(HCURSOR hCursor);

private:
	CJumpHook m_hkcurpos, m_hkshowcur, m_hksetcur;

	HWND m_hwnd;
	WNDPROC m_oldproc;
	WindowProc_Ctx m_ctx;

	static LRESULT WINAPI Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL WINAPI Hooked_SetCursorPos(int X, int Y);
	static int WINAPI Hooked_ShowCursor(BOOL bShow);
	static HCURSOR WINAPI Hooked_SetCursor(HCURSOR hCursor);
};

inline CWindowHook _g_windowhook;