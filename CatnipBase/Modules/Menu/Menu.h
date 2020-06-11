#pragma once
#include "Modules/Module.h"

class CMenu : public CModule
{
public:
	CMenu();
	int OnPresent();
	int OnWindowProc();

private:
	bool m_open;
};