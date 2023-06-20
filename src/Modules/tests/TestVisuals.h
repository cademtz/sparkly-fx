#pragma once

#include <Modules/BaseModule.h>
#include <imgui.h>
#include <SDK/vector.h>

class TestVisuals : public CModule
{
public:
	TestVisuals() { }
	void StartListening() override;

private:
	int OnMenu();
	int OnDraw();

	bool ShouldDraw() { return m_draw_players; }

	bool m_draw_players = false;
	bool m_draw_raytrace = false;
};