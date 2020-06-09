#include "Menu.h"
#include "Base/Base.h"
#include "Hooks/OverlayHook.h"
#include "Hooks/WindowProc.h"

#include <imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/imgui_demo.cpp>
#include "Base/imgui_impl_win32.h"

CMenu::CMenu()
{
	Listen(EVENT_DX9PRESENT, [this](void*) { OnPresent(); });
	Listen(EVENT_WINDOWPROC, [this](void*) { OnWindowProc(); });
}

void CMenu::OnPresent()
{
	auto hook = GETHOOK(COverlayHook);

	if (!ImGui::GetCurrentContext())
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;
		ImGui::GetIO().LogFilename = nullptr;

		ImGui_ImplDX9_Init(hook->Device());
		ImGui_ImplWin32_Init(Base::hWnd);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void CMenu::OnWindowProc()
{
	auto ctx = GETHOOK(CWindowHook)->Context();
	ImGui_ImplWin32_WndProcHandler(ctx.hwnd, ctx.msg, ctx.wparam, ctx.lparam);
}
