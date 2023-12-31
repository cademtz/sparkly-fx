#pragma once
#include "BaseModule.h"
#include <SDK/vector.h>
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

	void DrawText_Outline(const ImVec2& Pos, ImU32 Col, ImU32 Outline_Col, const char* Text_Begin, const char* Text_End = 0);
	bool DrawBox3D(const Vector& CornerA, const Vector& CornerB, const ImColor& Color, float Thickness = 1.f);
	bool DrawBox3D_Radius(const Vector& Point, const Vector& Radius, const ImColor& Color, float Thickness = 1.F) {
		return DrawBox3D(Point - Radius, Point + Radius, Color, Thickness);
	}
	bool DrawBox3D_Radius(const Vector& Point, float Radius, const ImColor& Color, float Thickness = 1.F) {
		return DrawBox3D(Point - Vector(Radius), Point + Vector(Radius), Color, Thickness);
	}

private:
	int OnPresent();
	int OnPaint();

	ImDrawList* m_list = nullptr;
	int m_frames = 0;
	std::mutex m_mtx;
};

inline CDraw gDraw;