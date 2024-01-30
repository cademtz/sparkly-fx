#include "Draw.h"
#include <Base/Interfaces.h>
#include <Hooks/OverlayHook.h>
#include <Hooks/PaintHook.h>
#include <Hooks/ClientHook.h>
#include <SDK/ienginevgui.h>
#include "Menu.h"

#include <imgui_internal.h>
#include <imgui/backends/imgui_impl_dx9.h>
#include <Base/imgui_impl_win32.h>
#include <SDK/vmatrix.h>

ImDrawData data;

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

		std::scoped_lock lock{m_mtx};
		m_list = new ImDrawList(ImGui::GetDrawListSharedData());
		data.DisplayPos = ImVec2(0, 0);
		data.CmdLists.push_back(m_list);
		data.CmdListsCount = 1;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	IDirect3DDevice9* device = g_hk_overlay.Device();

	IDirect3DSurface9* prev_depth_buffer = nullptr;
	IDirect3DStateBlock9* prev_state = nullptr;
	device->CreateStateBlock(D3DSBT_ALL, &prev_state);
	device->GetDepthStencilSurface(&prev_depth_buffer);

	device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
	device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	device->SetVertexShader(NULL);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS); // redundant due to ZENABLE
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
	device->SetDepthStencilSurface(nullptr);

	{
		std::scoped_lock lock{m_mtx};
		ImGui_ImplDX9_RenderDrawData(&data);
		m_frames = 0;
	}

	(void) OnImGui.DispatchEvent();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	prev_state->Apply();
	prev_state->Release();
	device->SetDepthStencilSurface(prev_depth_buffer);

	return 0;
}

int CDraw::OnPaint()
{
	std::scoped_lock lock{m_mtx};

	if (!m_list || m_frames > 0)
		return 0;

	m_list->_ResetForNewFrame();
	m_list->PushTextureID(ImGui::GetIO().Fonts[0].TexID);
	m_list->PushClipRectFullScreen();
	data.TotalVtxCount = m_list->VtxBuffer.Size;
	data.TotalIdxCount = m_list->IdxBuffer.Size;
	data.DisplaySize = ImVec2(m_list->_Data->ClipRectFullscreen.z, m_list->_Data->ClipRectFullscreen.w);

	(void) OnDraw.DispatchEvent();
	m_frames++;

	return 0;
}
