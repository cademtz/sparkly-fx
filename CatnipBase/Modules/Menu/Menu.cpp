#include "Menu.h"
#include "Base/Base.h"
#include "Hooks/OverlayHook.h"
#include "Hooks/WindowHook.h"

#include <imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/imgui_demo.cpp>
#include "Base/imgui_impl_win32.h"

CMenu::CMenu() : m_open(false)
{
	Listen(EVENT_DX9PRESENT, [this]() { return OnPresent(); });
	Listen(EVENT_WINDOWPROC, [this]() { return OnWindowProc(); });
}

int CMenu::OnPresent()
{
	if (!m_open)
		return 0;

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
	
	return 0;
}

int CMenu::OnWindowProc()
{
	auto ctx = GETHOOK(CWindowHook)->Context();
	switch (ctx.msg)
	{
	case WM_KEYDOWN:
		switch (ctx.wparam)
		{
		case VK_INSERT:
		case VK_F11:
			m_open = !m_open;
			return Return_NoOriginal | Return_Skip;
		}
	}

	if (m_open)
	{
		ctx.result = ImGui_ImplWin32_WndProcHandler(ctx.hwnd, ctx.msg, ctx.wparam, ctx.lparam);
		return Return_NoOriginal | Return_Skip;
	}
	return 0;
}
