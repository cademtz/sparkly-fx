#include "Menu.h"
#include <Base/Base.h>
#include <Hooks/WindowHook.h>
#include "Draw.h"
#include <SDK/convar.h>

#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/imgui_internal.h>
#include <Base/imgui_impl_win32.h>

CMenu::CMenu()
{
	RegisterEvent(EVENT_MENU);
	RegisterEvent(EVENT_POST_IMGUI_INPUT);
}

void CMenu::StartListening()
{
	Listen(EVENT_WINDOWPROC, [this] { return OnWindowProc(); });
	Listen(EVENT_IMGUI, [this]() { return OnImGui(); });
}

DWORD WINAPI UnhookThread(LPVOID)
{
	printf("Unhooking...\n");
	ConVar_Unregister();
	CBaseEvent::ShutdownAll();
	CBaseHook::UnHookAll();

	printf("Waiting for any hooked calls to end...\n");
	Sleep(0x1000);

	printf("FreeLibraryAndExitThread\n");
	FreeLibraryAndExitThread(Base::hInst, 0);

	return 0;
}

bool CMenu::AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_INSERT:
		case VK_F11:
			SetOpen(!IsOpen());
			g_hk_window.SetInputEnabled(IsOpen());
			return true;
		}
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	bool hide_input = PushEvent(EVENT_POST_IMGUI_INPUT) & Return_NoOriginal;
	return hide_input;
}

int CMenu::OnImGui()
{
	if (!IsOpen())
		return 0;

	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
	if (ImGui::Begin("Window"))
	{
		if (ImGui::Button("Eject"))
			CreateThread(0, 0, &UnhookThread, 0, 0, 0);

		PushEvent(EVENT_MENU);
	}
	ImGui::End();

	return 0;
}

int CMenu::OnWindowProc()
{
	WndProcArgs* ctx = g_hk_window.Context();
	if (AcceptMsg(ctx->hwnd, ctx->msg, ctx->lparam, ctx->wparam))
	{
		ctx->result = TRUE;
		return Return_NoOriginal | Return_Skip;
	}
	return 0;
}

bool CMenu::IsOpen() {
	return m_open;
}

void CMenu::SetOpen(bool Val) {
	m_open = Val;
}
