#pragma once
#include "Modules/Module.h"

class CMenu : public CModule
{
public:
	CMenu();
	int OnPresent();
	int OnWindowProc();
	int OnCurPos();
	int OnShowCur();
	int OnSetCur();

private:
	bool m_open = false, m_running = false;
};