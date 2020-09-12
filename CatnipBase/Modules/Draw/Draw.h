#pragma once
#include "Modules/Module.h"
#include <imgui.h>
#include <mutex>

DECL_EVENT(EVENT_IMGUI);
DECL_EVENT(EVENT_DRAW);

class Vector;

class CDraw : public CModule, CEventManager
{
public:
	CDraw();

	void StartListening() override;

	inline ImDrawList* List() { return m_list; }
	bool WorldToScreen(const Vector& World, ImVec2& Screen);

private:
	int OnPresent();
	int OnPaint();

	ImDrawList* m_list = nullptr;
	int m_frames = 0;
	std::mutex m_mtx;
};

inline CDraw gDraw;