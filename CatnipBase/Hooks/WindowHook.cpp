#include "WindowHook.h"

CWindowHook::CWindowHook() : m_hwnd(0), m_ctx(), BASEHOOK(CWindowHook) {
	RegisterEvent(EVENT_WINDOWPROC);
	RegisterEvent(EVENT_SETCURSORPOS);
}

void CWindowHook::Hook()
{
	m_hwnd = Base::hWnd;
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
	
	m_hkcurpos.Hook("win32u.dll", "NtUserSetCursorPos", Hooked_SetCursorPos);
	//m_hkshowcur.Hook("win32u.dll", "NtUserShowCursor", Hooked_ShowCursor);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
	m_hkcurpos.UnHook();
	m_hkshowcur.UnHook();
}

BOOL CWindowHook::SetCurPos(int X, int Y) {
	return m_hkcurpos.Original<decltype(Hooked_SetCursorPos)*>()(X, Y);
}
int CWindowHook::ShowCur(BOOL bShow) {
	return m_hkshowcur.Original<decltype(Hooked_ShowCursor)*>()(bShow);
}

LRESULT WINAPI CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static auto hook = GETHOOK(CWindowHook);
	hook->Context() = WindowProc_Ctx{ hWnd, uMsg, wParam, lParam, TRUE };

	int flags = hook->PushEvent(EVENT_WINDOWPROC);
	if (flags & Return_NoOriginal)
		return hook->Context().result;

	return hook->OldProc()(hWnd, uMsg, wParam, lParam);
}

BOOL CWindowHook::Hooked_SetCursorPos(int X, int Y)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();
	ctx.cur_x = X, ctx.cur_y = Y;

	int flags = hook->PushEvent(EVENT_SETCURSORPOS);
	if (flags & Return_NoOriginal)
		return TRUE;

	return hook->SetCurPos(ctx.cur_x, ctx.cur_y);
}

int __stdcall CWindowHook::Hooked_ShowCursor(BOOL bShow)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();

	/*static CURSORINFO info{ sizeof(info) };
	GetCursorInfo(&info);
	BOOL test = info.flags & CURSOR_SHOWING;
	hook->ShowCur(test);
	int displayCount = hook->ShowCur(!test);*/

	int flags = hook->PushEvent(EVENT_SHOWCURSOR);
	if (flags & Return_NoOriginal)
		return bShow ? -1 : 0; // Prevent some games from infinitely looping to get desired display count

	return hook->SetCurPos(ctx.cur_x, ctx.cur_y);
}
