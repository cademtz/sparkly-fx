#pragma once
#include "BaseModule.h"
#include <SDK/vector.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <mutex>
#include <Hooks/PaintHook.h>

class Vector;

class CDraw : public CModule
{
public:
	void StartListening() override;

	ImDrawList* List() { return &m_imdrawlist; }
	bool WorldToScreen(const Vector& World, ImVec2& Screen);

	void DrawText_Outline(const ImVec2& Pos, ImU32 Col, ImU32 Outline_Col, const char* Text_Begin, const char* Text_End = 0);
	bool DrawBox3D(const Vector& CornerA, const Vector& CornerB, const ImColor& Color, float Thickness = 1.f);
	bool DrawBox3D_Radius(const Vector& Point, const Vector& Radius, const ImColor& Color, float Thickness = 1.F) {
		return DrawBox3D(Point - Radius, Point + Radius, Color, Thickness);
	}
	bool DrawBox3D_Radius(const Vector& Point, float Radius, const ImColor& Color, float Thickness = 1.F) {
		return DrawBox3D(Point - Vector(Radius), Point + Vector(Radius), Color, Thickness);
	}

	using ImGuiEvent = EventSource<void()>;
	using DrawEvent = EventSource<void()>;

	static inline ImGuiEvent OnImGui;
	static inline DrawEvent OnDraw;
private:
	int OnPresent();
	int OnPaint();

	ImDrawData m_imdrawdata;
	ImDrawListSharedData m_imdrawlist_shared_data;
	ImDrawList m_imdrawlist{&m_imdrawlist_shared_data};
	bool m_is_drawlist_ready = false;
	int m_frames = 0;
	std::mutex m_mtx;
};

inline CDraw gDraw;