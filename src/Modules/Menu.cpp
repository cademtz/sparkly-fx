#include "Menu.h"
#include <Base/Base.h>
#include <Hooks/WindowHook.h>
#include "Draw.h"

#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/imgui_internal.h>
#include <Base/imgui_impl_win32.h>

CMenu::CMenu()
{
}

void CMenu::StartListening()
{
	CDraw::OnImGui.ListenNoArgs(&CMenu::OnImGui, this);
	CWindowHook::OnWndProc.Listen(&CMenu::OnWindowProc, this);
}

DWORD WINAPI UnhookThread(LPVOID)
{
	printf("Unhooking...\n");
	//todo
	EventSource<void()>::Shutdown();
	CBaseHook::UnHookAll();

	printf("Waiting for any hooked calls to end...\n");
	Sleep(0x1000);

	printf("FreeLibraryAndExitThread\n");
	FreeLibraryAndExitThread(Base::hInst, 0);

	return 0;
}

bool CMenu::AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_INSERT:
		case VK_F11:
			SetOpen(!IsOpen());
			g_hk_window.SetInputEnabled(IsOpen());
			return true;
		default: break;
		}
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	const auto& res = OnPostImguiInput.DispatchEvent();
	return res.Flags & EventReturnFlags::NoOriginal;
}

void CMenu::OnImGui() const
{
	if (!IsOpen())
		return;

	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
	if (ImGui::Begin("Window"))
	{
		if (ImGui::Button("Eject"))
			CreateThread(0, 0, &UnhookThread, 0, 0, 0);

		(void) OnMenu.DispatchEvent();
	}
	ImGui::End();
}

void CMenu::OnWindowProc(CWindowHook::WndProcEvent::Param& p, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (AcceptMsg(hWnd, uMsg, lParam, wParam))
	{
		p.ReturnValue = TRUE;
		p.Flags |= (EventReturnFlags::NoOriginal | EventReturnFlags::Skip);
	}
}

bool CMenu::IsOpen() const {
	return m_open;
}

void CMenu::SetOpen(const bool Val) {
	m_open = Val;
}
