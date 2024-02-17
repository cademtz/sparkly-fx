#include "WindowHook.h"
#include <Base/Base.h>

void CWindowHook::Hook()
{
	m_hwnd = Base::hWnd;
	m_oldproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Hooked_WndProc)));
	
	m_hkcurpos.Hook(&::SetCursorPos, &Hooked_SetCursorPos);
	m_hkshowcur.Hook(&::ShowCursor, &Hooked_ShowCursor);
	m_hksetcur.Hook(&::SetCursor, &Hooked_SetCursor);
	m_hkgetcurpos.Hook(&::GetCursorPos, &Hooked_GetCursorPos);
	m_hk_getcurinfo.Hook(&::GetCursorInfo, &Hooked_GetCursorInfo);
}

void CWindowHook::Unhook()
{
	SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_oldproc));
	m_hkcurpos.Unhook();
	m_hkshowcur.Unhook();
	m_hksetcur.Unhook();
	m_hkgetcurpos.Unhook();
	m_hk_getcurinfo.Unhook();
}

BOOL CWindowHook::SetCurPos(int X, int Y)
{
	static auto NtUserSetCursorPos = static_cast<decltype(Hooked_SetCursorPos)*>(m_hkcurpos.Original());
	return NtUserSetCursorPos(X, Y);
}
int CWindowHook::ShowCur(BOOL bShow) {
	static auto NtUserShowCursor = static_cast<decltype(Hooked_ShowCursor)*>(m_hkshowcur.Original());
	return NtUserShowCursor(bShow);
}

HCURSOR CWindowHook::SetCur(HCURSOR hCursor)
{
	auto NtUserSetCursor = static_cast<decltype(Hooked_SetCursor)*>(m_hksetcur.Original());
	return NtUserSetCursor(hCursor);
}

BOOL CWindowHook::GetCurPos(LPPOINT Point)
{
	auto getcurpos = static_cast<decltype(::GetCursorPos)*>(m_hkgetcurpos.Original());
	return getcurpos(Point);
}

BOOL CWindowHook::GetCurInfo(PCURSORINFO pci)
{
	auto getcurinfo = static_cast<decltype(::GetCursorInfo)*>(m_hk_getcurinfo.Original());
	return getcurinfo(pci);
}

void CWindowHook::SetInputEnabled(bool Enabled)
{
	if (Enabled != GetInputEnabled())
	{
		if (Enabled)
		{
			m_lastInput.cursor_info.cbSize = sizeof(m_lastInput.cursor_info);
			if (!this->GetCurInfo(&m_lastInput.cursor_info))
				MessageBoxA(0, "Failed to get cursor info", "Warning", MB_ICONWARNING);
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

LRESULT WINAPI CWindowHook::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = TRUE;
	int flags = OnWndProc.DispatchEvent(result, hWnd, uMsg, wParam, lParam);
	if (flags & EventReturnFlags::NoOriginal)
		return result;
	
	if (g_hk_window.GetInputEnabled())
	{
		switch (uMsg)
		{
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_MOUSEHOVER:
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_NCMOUSELEAVE:
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_CHAR:
			return true;
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			//break; // Prevent "stuck" input by allowing game to recieve key up
			// For now, this is more safe...
			// Some games/UIs will use mouse-/key-up events to trigger some actions. 
			return true;
		default: break;
		}
	}

	return CallWindowProc(g_hk_window.OldProc(), hWnd, uMsg, wParam, lParam);
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
	
	return g_hk_window.SetCur(hCursor);
}

BOOL __stdcall CWindowHook::Hooked_GetCursorPos(LPPOINT Point)
{
	if (g_hk_window.GetInputEnabled())
	{
		const auto& cursor = g_hk_window.m_lastInput.cursor_info;
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
