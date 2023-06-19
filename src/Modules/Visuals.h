#pragma once

#include "BaseModule.h"
#include <imgui.h>
#include <SDK/vector.h>

class CVisuals : public CModule
{
public:
	CVisuals() { }
	void StartListening() override;

private:
	int OnMenu();
	int OnDraw();

	bool ShouldDraw() { return m_draw_players; }

	bool m_draw_players = false;
	bool m_draw_raytrace = false;
};

inline CVisuals _g_esp;