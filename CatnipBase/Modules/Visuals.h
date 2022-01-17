#pragma once

#include "BaseModule.h"

class CVisuals : public CModule
{
public:
	CVisuals() { }
	void StartListening() override;

private:
	int OnDraw();
};

inline CVisuals _g_esp;