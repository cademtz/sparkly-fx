#include "WindowProc.h"

CWindowHook::CWindowHook(HWND Window) : m_hwnd(Window), BASEHOOK(CWindowHook)
{
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
}

LRESULT __stdcall CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto hook = GETHOOK(CWindowHook);
	return hook->OldProc()(hWnd, uMsg, wParam, lParam);
}
