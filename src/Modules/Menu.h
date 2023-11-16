#pragma once
#include "BaseModule.h"
#include <imgui.h>

DECL_EVENT(EVENT_MENU);
/// @brief This runs after IMGUI reads input. It can also hide input from the game.
DECL_EVENT(EVENT_POST_IMGUI_INPUT);

class CMenu : public CModule, CEventManager
{
public:
	CMenu();
	void StartListening() override;
	void SetOpen(bool Val);
	bool IsOpen();

private:
	bool AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam);

	int OnImGui();
	int OnWindowProc();

	bool m_open = false;
};

inline CMenu g_menu;