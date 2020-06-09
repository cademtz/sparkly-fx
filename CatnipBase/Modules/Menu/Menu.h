#pragma once
#include "Modules/Module.h"

class CMenu : public CModule
{
public:
	CMenu();
	void OnPresent();
	void OnWindowProc();
};