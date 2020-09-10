#include "WindowHook.h"

CWindowHook::CWindowHook() : m_hwnd(0), m_ctx(), BASEHOOK(CWindowHook) {
	RegisterEvent(EVENT_WINDOWPROC);
	RegisterEvent(EVENT_SETCURSORPOS);
	RegisterEvent(EVENT_SHOWCURSOR);
	RegisterEvent(EVENT_SETCURSOR);
}

void CWindowHook::Hook()
{
	m_hwnd = Base::hWnd;
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
	
	m_hkcurpos.Hook(&SetCursorPos, Hooked_SetCursorPos);
	//m_hkshowcur.Hook(&ShowCursor, Hooked_ShowCursor); // Steam overlay freaks out and locks up. idk im tired.
	m_hksetcur.Hook("win32u.dll", "NtUserSetCursor", Hooked_SetCursor);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
	m_hkcurpos.UnHook();
	m_hkshowcur.UnHook();
	m_hksetcur.UnHook();
}

BOOL CWindowHook::SetCurPos(int X, int Y)
{
	static auto NtUserSetCursorPos = (decltype(Hooked_SetCursorPos)*)Base::GetProc(Base::GetModule("win32u.dll"), "NtUserSetCursorPos");
	return NtUserSetCursorPos(X, Y);
}
int CWindowHook::ShowCur(BOOL bShow) {
	static auto NtUserShowCursor = (decltype(Hooked_ShowCursor)*)Base::GetProc(Base::GetModule("win32u.dll"), "NtUserShowCursor");
	return NtUserShowCursor(bShow);
}

HCURSOR CWindowHook::SetCur(HCURSOR hCursor)
{
	auto NtUserSetCursor = (decltype(Hooked_SetCursor)*)m_hksetcur.Original();
	return NtUserSetCursor(hCursor);
}

LRESULT WINAPI CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();
	ctx->hwnd = hWnd, ctx->msg = uMsg, ctx->wparam = wParam, ctx->lparam = lParam, ctx->result = TRUE;

	int flags = hook->PushEvent(EVENT_WINDOWPROC);
	if (flags & Return_NoOriginal)
		return ctx->result;

	return hook->OldProc()(hWnd, uMsg, wParam, lParam);
}

BOOL CWindowHook::Hooked_SetCursorPos(int X, int Y)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();
	ctx->cur_x = X, ctx->cur_y = Y;

	int flags = hook->PushEvent(EVENT_SETCURSORPOS);
	if (flags & Return_NoOriginal)
		return TRUE;

	return hook->SetCurPos(ctx->cur_x, ctx->cur_y);
}

int CWindowHook::Hooked_ShowCursor(BOOL bShow)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();

	int flags = hook->PushEvent(EVENT_SHOWCURSOR);
	if (flags & Return_NoOriginal)
		return bShow ? -1 : 0; // Prevent some games from infinitely looping to get desired display count

	return hook->SetCurPos(ctx->cur_x, ctx->cur_y);
}

HCURSOR CWindowHook::Hooked_SetCursor(HCURSOR hCursor)
{
	static auto hook = GETHOOK(CWindowHook);
	auto ctx = hook->Context();
	ctx->hCursor = hCursor, ctx->hPrevCursor = GetCursor();

	int flags = hook->PushEvent(EVENT_SETCURSOR);
	if (flags & Return_NoOriginal)
		return ctx->hPrevCursor;

	return hook->SetCur(hCursor);
}
