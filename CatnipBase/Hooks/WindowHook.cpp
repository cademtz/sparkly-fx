#include "WindowHook.h"

CWindowHook::CWindowHook() : m_hwnd(0), m_ctx(), BASEHOOK(CWindowHook) {
	RegisterEvent(EVENT_WINDOWPROC);
}

void CWindowHook::Hook()
{
	m_hwnd = Base::hWnd;
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
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
