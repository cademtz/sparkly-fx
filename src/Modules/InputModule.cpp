#include "InputModule.h"
#include <Base/Base.h>
#include <Hooks/WindowHook.h>
#include "Draw.h"

#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/imgui_internal.h>
#include <Base/imgui_impl_win32.h>

InputModule::InputModule() {}

void InputModule::StartListening() {
	CWindowHook::OnWndProc.Listen(&InputModule::OnWindowProc, this);
}

bool InputModule::AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_INSERT:
		case VK_F11:
			SetOverlayOpen(!IsOverlayOpen());
			g_hk_window.SetInputEnabled(IsOverlayOpen());
			return true;
		default: break;
		}
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	int flags = OnPostImguiInput.DispatchEvent();
	return flags & EventReturnFlags::NoOriginal;
}

int InputModule::OnWindowProc(LRESULT& result, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (AcceptMsg(hWnd, uMsg, lParam, wParam))
	{
		result = TRUE;
		return EventReturnFlags::NoOriginal | EventReturnFlags::Skip;
	}
	return 0;
}

bool InputModule::IsOverlayOpen() const {
	return m_open;
}

void InputModule::SetOverlayOpen(const bool Val) {
	m_open = Val;
}
