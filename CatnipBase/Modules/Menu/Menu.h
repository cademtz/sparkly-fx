#pragma once
#include "Modules/Module.h"
#include <imgui.h>

DECL_EVENT(EVENT_MENU);

class CMenu : public CModule, CEventManager
{
public:
	CMenu();
	void StartListening() override;

private:
	bool AcceptMsg(HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam);

	int OnImGui();
	int OnWindowProc();

	bool IsOpen();
	void SetOpen(bool Val);

	bool m_open = false;
};

inline CMenu g_menu;