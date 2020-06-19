#pragma once
#include "Modules/Module.h"

class CMenu : public CModule
{
public:
	CMenu() { }
	void StartListening() override;

private:
	int OnImGui();
	int OnWindowProc();
	int OnCurPos();
	int OnShowCur();
	int OnSetCur();

	bool m_open = false, m_running = false;
};

inline CMenu _g_menu;