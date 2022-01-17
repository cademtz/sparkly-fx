#pragma once
#include <Windows.h>
#include "Hooks.h"

DECL_EVENT(EVENT_WINDOWPROC);

struct WndProcArgs
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
	CWindowHook();

	void Hook() override;
	void Unhook() override;

	inline HWND Window() const { return m_hwnd; }
	inline WNDPROC OldProc() const { return m_oldproc; }
	inline WndProcArgs* Context() { return &m_ctx; }

	BOOL SetCurPos(int X, int Y);
	int ShowCur(BOOL bShow);
	HCURSOR SetCur(HCURSOR hCursor);
	BOOL GetCurPos(LPPOINT Point);
	BOOL GetCurInfo(PCURSORINFO pci);

	void SetInputEnabled(bool Enabled);
	bool GetInputEnabled() { return !m_passInput && !m_inputBypai; }

protected:
	friend class CEnableInput;
	void SetInputEnabled_Increment(bool Enabled);

private:
	CJumpHook m_hkcurpos, m_hkshowcur, m_hksetcur, m_hkgetcurpos, m_hk_getcurinfo, m_hkgetmsg;

	HWND m_hwnd;
	WNDPROC m_oldproc;
	int m_inputBypai = 0; // Should be thread-local, if used in the future
	bool m_passInput = true;

	struct
	{
		CURSORINFO cursor_info = { sizeof(cursor_info) };
		int show_count;
	} m_lastInput = { 0 };

	static WndProcArgs m_ctx;

	static LRESULT WINAPI Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL WINAPI Hooked_SetCursorPos(int X, int Y);
	static int WINAPI Hooked_ShowCursor(BOOL bShow);
	static HCURSOR WINAPI Hooked_SetCursor(HCURSOR hCursor);
	static BOOL WINAPI Hooked_GetCursorPos(LPPOINT Point);
	static BOOL WINAPI Hooked_GetCursorInfo(PCURSORINFO pci);
};

inline CWindowHook g_hk_window;

class CEnableInput {
public:
	CEnableInput() { g_hk_window.SetInputEnabled_Increment(false); }
	~CEnableInput() { g_hk_window.SetInputEnabled_Increment(true); }
};
