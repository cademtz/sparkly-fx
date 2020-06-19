#include "Menu.h"
#include "Base/Base.h"
#include "Hooks/WindowHook.h"
#include "Modules/Draw/Draw.h"

#include <imgui.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include <imgui/imgui_demo.cpp>
#include <imgui/imgui_internal.h>
#include "Base/imgui_impl_win32.h"

void CMenu::StartListening()
{
	Listen(EVENT_WINDOWPROC, [this]() { return OnWindowProc(); });
	Listen(EVENT_SETCURSORPOS, [this]() { return OnCurPos(); });
	Listen(EVENT_SHOWCURSOR, [this]() { return OnShowCur();  });
	Listen(EVENT_SETCURSOR, [this]() { return OnSetCur(); });
	Listen(EVENT_IMGUI, [this]() { return OnImGui(); });
}

int CMenu::OnImGui()
{
	if (!m_open)
		return 0;

	ImGui_ImplWin32_NewFrame();
	ImGui::ShowDemoWindow();

	return 0;
}

int CMenu::OnWindowProc()
{
	auto ctx = GETHOOK(CWindowHook)->Context();
	switch (ctx->msg)
	{
	case WM_KEYDOWN:
		switch (ctx->wparam)
		{
		case VK_INSERT:
		case VK_F11:
			m_open = !m_open;
			return Return_NoOriginal | Return_Skip;
		}
	}

	if (m_open)
	{
		m_running = true;
		ImGui_ImplWin32_WndProcHandler(ctx->hwnd, ctx->msg, ctx->wparam, ctx->lparam);
		m_running = false;

		switch (ctx->msg)
		{
		case WM_KEYDOWN:
			if (!ImGui::GetIO().WantTextInput)
				return 0; // Let users continue walking in-game unless typing inside IMGUI
		case WM_KEYUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
			return 0; // Prevent "stuck" input by allowing game to recieve key up
		}
		return Return_NoOriginal | Return_Skip;
	}
	return 0;
}

int CMenu::OnCurPos()
{
	if (m_open)
		return Return_NoOriginal;
	return 0;
}

int CMenu::OnShowCur()
{
	if (m_open)
	{
		static auto hook = GETHOOK(CWindowHook);
		static CURSORINFO info{ sizeof(info) };
		GetCursorInfo(&info);
		if (!(info.flags & CURSOR_SHOWING))
			while (ShowCursor(true) < 0);

		return Return_NoOriginal;
	}
	return 0;
}

int CMenu::OnSetCur()
{
	if (!m_open || m_running)
		return 0;
	return Return_NoOriginal;
}
