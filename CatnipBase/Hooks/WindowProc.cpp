#include "WindowProc.h"

CWindowHook::CWindowHook(HWND Window) : m_hwnd(Window), m_ctx(), BASEHOOK(CWindowHook)
{
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
	RegisterEvent(EVENT_WINDOWPROC);
}

void CWindowHook::Unhook() {
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
}

LRESULT WINAPI CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto hook = GETHOOK(CWindowHook);
	hook->Context() = WindowProc_Ctx{ hWnd, uMsg, wParam, lParam };
	hook->PushEvent(EVENT_WINDOWPROC, 0);
	return hook->OldProc()(hWnd, uMsg, wParam, lParam);
}
