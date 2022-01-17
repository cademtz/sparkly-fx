#include "WindowHook.h"
#include <assert.h>

WndProcArgs CWindowHook::m_ctx;

CWindowHook::CWindowHook() : m_hwnd(0), BASEHOOK(CWindowHook)
{
	RegisterEvent(EVENT_WINDOWPROC);
}

void CWindowHook::Hook()
{
	m_hwnd = Base::hWnd;
	m_oldproc = (WNDPROC)SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);
	
	m_hkcurpos.Hook(&::SetCursorPos, &Hooked_SetCursorPos);
	m_hkshowcur.Hook(&::ShowCursor, &Hooked_ShowCursor);
	m_hksetcur.Hook(&::SetCursor, &Hooked_SetCursor);
	m_hkgetcurpos.Hook(&::GetCursorPos, &Hooked_GetCursorPos);
	m_hk_getcurinfo.Hook(&::GetCursorInfo, &Hooked_GetCursorInfo);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_oldproc);
	m_hkcurpos.UnHook();
	m_hkshowcur.UnHook();
	m_hksetcur.UnHook();
	m_hkgetcurpos.UnHook();
	m_hk_getcurinfo.UnHook();
}

BOOL CWindowHook::SetCurPos(int X, int Y)
{
	static auto NtUserSetCursorPos = (decltype(Hooked_SetCursorPos)*)m_hkcurpos.Original();
	return NtUserSetCursorPos(X, Y);
}
int CWindowHook::ShowCur(BOOL bShow) {
	static auto NtUserShowCursor = (decltype(Hooked_ShowCursor)*)m_hkshowcur.Original();
	return NtUserShowCursor(bShow);
}

HCURSOR CWindowHook::SetCur(HCURSOR hCursor)
{
	auto NtUserSetCursor = (decltype(Hooked_SetCursor)*)m_hksetcur.Original();
	return NtUserSetCursor(hCursor);
}

BOOL CWindowHook::GetCurPos(LPPOINT Point)
{
	auto getcurpos = (decltype(::GetCursorPos)*)m_hkgetcurpos.Original();
	return getcurpos(Point);
}

BOOL CWindowHook::GetCurInfo(PCURSORINFO pci)
{
	auto getcurinfo = (decltype(::GetCursorInfo)*)m_hk_getcurinfo.Original();
	return getcurinfo(pci);
}

void CWindowHook::SetInputEnabled(bool Enabled)
{
	if (Enabled != GetInputEnabled())
	{
		if (Enabled)
		{
			this->GetCurInfo(&m_lastInput.cursor_info);
			this->SetCur(LoadCursor(NULL, IDC_ARROW));

			m_lastInput.show_count = this->ShowCur(true) - 1;

			if (m_lastInput.show_count + 1 < 0)
				while (this->ShowCur(true) < 0);
		}
		else // Restore mouse settings
		{
			this->SetCurPos(m_lastInput.cursor_info.ptScreenPos.x, m_lastInput.cursor_info.ptScreenPos.y);

			int show_count = this->ShowCur(m_lastInput.show_count >= 0);
			while (show_count != m_lastInput.show_count)
				show_count = this->ShowCur(show_count < m_lastInput.show_count ? true : false);

			this->SetCur(m_lastInput.cursor_info.hCursor);
		}
	}

	m_passInput = !Enabled;
}

void CWindowHook::SetInputEnabled_Increment(bool Enabled)
{
	m_inputBypai += Enabled ? -1 : 1;
	assert(m_inputBypai >= 0);
}

LRESULT WINAPI CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto ctx = g_hk_window.Context();
	ctx->hwnd = hWnd, ctx->msg = uMsg, ctx->wparam = wParam, ctx->lparam = lParam, ctx->result = TRUE;

	int flags = g_hk_window.PushEvent(EVENT_WINDOWPROC);
	if (flags & Return_NoOriginal)
		return ctx->result;
	else if (g_hk_window.GetInputEnabled())
		return true;

	return g_hk_window.OldProc()(hWnd, uMsg, wParam, lParam);
}

BOOL CWindowHook::Hooked_SetCursorPos(int X, int Y)
{
	if (g_hk_window.GetInputEnabled())
	{
		auto& cursor = g_hk_window.m_lastInput.cursor_info;
		cursor.ptScreenPos.x = X;
		cursor.ptScreenPos.y = Y;
		return true;
	}

	return g_hk_window.SetCurPos(X, Y);
}

int CWindowHook::Hooked_ShowCursor(BOOL bShow)
{
	if (g_hk_window.GetInputEnabled())
	{
		auto& cursor = g_hk_window.m_lastInput;
		cursor.show_count += bShow ? 1 : -1;

		if (cursor.show_count >= 0)
			cursor.cursor_info.flags |= CURSOR_SHOWING;
		else
			cursor.cursor_info.flags &= ~CURSOR_SHOWING;

		return cursor.show_count;
	}

	return g_hk_window.ShowCur(bShow);
}

HCURSOR CWindowHook::Hooked_SetCursor(HCURSOR hCursor)
{
	if (g_hk_window.GetInputEnabled())
	{
		auto& cursor = g_hk_window.m_lastInput.cursor_info;
		cursor.hCursor = hCursor;
		return hCursor;
	}

	if (g_hk_window.GetInputEnabled())
		printf("Farded\n");
	return g_hk_window.SetCur(hCursor);
}

BOOL __stdcall CWindowHook::Hooked_GetCursorPos(LPPOINT Point)
{
	if (g_hk_window.GetInputEnabled())
	{
		auto& cursor = g_hk_window.m_lastInput.cursor_info;
		*Point = cursor.ptScreenPos;
		return true;
	}

	return g_hk_window.GetCurPos(Point);
}

BOOL __stdcall CWindowHook::Hooked_GetCursorInfo(PCURSORINFO pci)
{
	if (g_hk_window.GetInputEnabled())
	{
		*pci = g_hk_window.m_lastInput.cursor_info;
		return true;
	}

	return g_hk_window.GetCurInfo(pci);
}
