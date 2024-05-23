#include "Draw.h"
#include <Base/Base.h>
#include <Base/Interfaces.h>
#include <Hooks/OverlayHook.h>
#include <Hooks/PaintHook.h>
#include <SDK/ienginevgui.h>

#include <imgui_internal.h>
#include <imgui/backends/imgui_impl_dx9.h>
#include <Base/imgui_impl_win32.h>
#include <SDK/vmatrix.h>

void CDraw::StartListening()
{
	COverlayHook::OnPresent.ListenNoArgs(&CDraw::OnPresent, this);
	CPaintHook::OnPaint.ListenNoArgs(&CDraw::OnPaint, this);
}

bool CDraw::WorldToScreen(const Vector& World, ImVec2& Screen)
{
	int width, height;

	typedef float matrix4x4_t[4][4];
	const matrix4x4_t& vmatrix = (matrix4x4_t&)Interfaces::engine->WorldToScreenMatrix();
	Interfaces::engine->GetScreenSize(width, height);

	float w = vmatrix[3][0] * World[0] + vmatrix[3][1] * World[1] + vmatrix[3][2] * World[2] + vmatrix[3][3];
	if (w > 0.001)
	{
		float f = 1 / w;
		Screen.x = (width / 2) + (0.5 * ((vmatrix[0][0] * World[0] + vmatrix[0][1] * World[1] + vmatrix[0][2] * World[2] + vmatrix[0][3]) * f) * width + 0.5);
		Screen.y = (height / 2) - (0.5 * ((vmatrix[1][0] * World[0] + vmatrix[1][1] * World[1] + vmatrix[1][2] * World[2] + vmatrix[1][3]) * f) * height + 0.5);
		return true;
	}

	return false;
}

void CDraw::DrawText_Outline(const ImVec2& Pos, ImU32 Col, ImU32 Outline_Col, const char* Text_Begin, const char* Text_End)
{
	List()->AddText(ImVec2(Pos.x + 1, Pos.y), Outline_Col, Text_Begin, Text_End);
	List()->AddText(ImVec2(Pos.x - 1, Pos.y), Outline_Col, Text_Begin, Text_End);
	List()->AddText(ImVec2(Pos.x, Pos.y + 1), Outline_Col, Text_Begin, Text_End);
	List()->AddText(ImVec2(Pos.x, Pos.y - 1), Outline_Col, Text_Begin, Text_End);
	List()->AddText(Pos, Col, Text_Begin, Text_End);
}

bool CDraw::DrawBox3D(const Vector& CornerA, const Vector& CornerB, const ImColor& Color, float Thickness)
{
	Vector vmin = Vector(min(CornerA.x, CornerB.x), min(CornerA.y, CornerB.y), min(CornerA.z, CornerB.z));
	Vector vdiff = Vector(max(CornerA.x, CornerB.x), max(CornerA.y, CornerB.y), max(CornerA.z, CornerB.z));
	vdiff -= vmin;

	// Bottom corners in clockwise order
	Vector corners[4] =
	{
		{ vmin.x, vmin.y, vmin.z },
		{ vmin.x, vmin.y + vdiff.y, vmin.z },
		{ vmin.x + vdiff.x, vmin.y + vdiff.y, vmin.z },
		{ vmin.x + vdiff.x, vmin.y, vmin.z },
	};
	ImVec2 screen[8];

	for (int i = 0; i < 4; i++) // Get all screen positions
	{
		if (!WorldToScreen(corners[i], screen[i]))
			return false;

		corners[i].z += vdiff.z; // Add height, creating top corner
		if (!WorldToScreen(corners[i], screen[i + 4]))
			return false;
	}

	for (int i = 0; i < 4; i++) // Another loop to connect them with lines
	{
		int next = i == 3 ? -3 : 1;
		List()->AddLine(screen[i], screen[i + next], Color, Thickness);
		List()->AddLine(screen[i], screen[i + 4], Color, Thickness);
		List()->AddLine(screen[i + 4], screen[i + 4 + next], Color, Thickness);
	}

	return true;
}

int CDraw::OnPresent()
{
	if (!ImGui::GetCurrentContext())
	{
		auto ctx = ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;
		ImGui::GetIO().LogFilename = nullptr;

		ImGui_ImplDX9_Init(g_hk_overlay.Device());
		ImGui_ImplWin32_Init(Base::hWnd);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (!m_is_drawlist_ready)
	{
		m_imdrawlist_shared_data = *ImGui::GetDrawListSharedData();
		m_is_drawlist_ready = true;
	}

	IDirect3DDevice9* device = g_hk_overlay.Device();

	IDirect3DSurface9* prev_depth_buffer = nullptr;
	device->GetDepthStencilSurface(&prev_depth_buffer);
	device->SetDepthStencilSurface(nullptr);

	{
		std::scoped_lock lock{m_mtx};
		ImGui_ImplDX9_RenderDrawData(&m_imdrawdata);
		m_frames = 0;
	}

	(void) OnImGui.DispatchEvent();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	device->SetDepthStencilSurface(prev_depth_buffer);

	return 0;
}

int CDraw::OnPaint()
{
	if (!m_is_drawlist_ready)
		return 0;

	std::scoped_lock lock{m_mtx};
	if (m_frames > 0)
		return 0;

	m_imdrawlist._ResetForNewFrame();
	m_imdrawlist.PushTextureID(ImGui::GetIO().Fonts[0].TexID);
	m_imdrawlist.PushClipRectFullScreen();

	(void) OnDraw.DispatchEvent();
	m_imdrawdata.Clear();
	m_imdrawdata.AddDrawList(&m_imdrawlist);
	m_imdrawdata.Valid = true;
	m_imdrawdata.DisplayPos = ImGui::GetMainViewport()->Pos;
	m_imdrawdata.DisplaySize = ImGui::GetMainViewport()->Size;
	m_imdrawdata.FramebufferScale = ImVec2(1,1);
	m_frames++;

	return 0;
}
