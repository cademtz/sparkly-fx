#include "Draw.h"
#include "Base/Interfaces.h"
#include "Hooks/OverlayHook.h"
#include "Hooks/PaintHook.h"
#include "Hooks/ClientHook.h"
#include "SDK/ienginevgui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/examples/imgui_impl_dx9.h>
#include "Base/imgui_impl_win32.h"

ImDrawData data;

CDraw::CDraw()
{
	RegisterEvent(EVENT_IMGUI);
	RegisterEvent(EVENT_DRAW);
}

void CDraw::StartListening()
{
	Listen(EVENT_DX9PRESENT, [this] { return OnPresent(); });
	Listen(EVENT_PAINT, [this] { return OnPaint(); });
}

typedef float Urinate[3][4];

bool CDraw::WorldToScreen(const Vector& World, ImVec2& Screen)
{
	int width, height;
	const Urinate& vmatrix = (Urinate&)Interfaces::engine->WorldToScreenMatrix();
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

int CDraw::OnPresent()
{
	static auto hook = GETHOOK(COverlayHook);

	if (!ImGui::GetCurrentContext())
	{
		auto ctx = ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;
		ImGui::GetIO().LogFilename = nullptr;

		ImGui_ImplDX9_Init(hook->Device());
		ImGui_ImplWin32_Init(Base::hWnd);

		m_mtx.lock();

		m_list = new ImDrawList(ImGui::GetDrawListSharedData());
		data.DisplayPos = ImVec2(0, 0);
		data.CmdLists = &m_list;
		data.CmdListsCount = 1;

		m_mtx.unlock();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_mtx.lock();
	if (m_frames)
	{
		char buf[16];
		itoa(m_frames, buf, 10);
		m_list->AddText(ImVec2(25, 25), ImColor(255, 0, 0), buf);
	}
	ImGui_ImplDX9_RenderDrawData(&data);
	m_frames = 0;
	m_mtx.unlock();

	PushEvent(EVENT_IMGUI);
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());


	return 0;
}

int CDraw::OnPaint()
{
	m_mtx.lock();

	static auto hook = GETHOOK(CPaintHook);
	auto ctx = hook->Context();

	if (!m_list || m_frames > 0)
	{
		m_mtx.unlock();
		return 0;
	}

	m_list->_ResetForNewFrame();
	m_list->PushTextureID(ImGui::GetIO().Fonts[0].TexID);
	m_list->PushClipRectFullScreen();
	m_list->AddRectFilled(ImVec2(25, 25), ImVec2(100, 100), ImColor(0, 128, 255));
	data.TotalVtxCount = m_list->VtxBuffer.Size;
	data.TotalIdxCount = m_list->IdxBuffer.Size;
	data.DisplaySize = ImVec2(m_list->_Data->ClipRectFullscreen.z, m_list->_Data->ClipRectFullscreen.w);

	PushEvent(EVENT_DRAW);
	m_frames++;

	m_mtx.unlock();
	return 0;
}
