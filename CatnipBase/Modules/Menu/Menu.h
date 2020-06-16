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

private:
	bool m_open;
};