#include "PaintHook.h"
#include <Base/Interfaces.h>

// We are currently avoiding a paint traverse hook because x64 GMod sux

#define OFF_PAINTTRAVERSE 41

void CPaintHook::Hook()
{
	m_vguihook.Hook(Interfaces::vgui->Inst());
	m_vguihook.Set(Interfaces::vgui->GetOffset(Off_Paint), &Hooked_Paint);
}

void CPaintHook::Unhook() {
	m_vguihook.Unhook();
}

void CPaintHook::PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	using Fn = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);
	static auto original = m_vguihook.Get<Fn>(OFF_PAINTTRAVERSE);
	return original(Interfaces::panels, vguiPanel, forceRepaint, allowForce);
}

void CPaintHook::Paint(PaintMode_t mode)
{
	using Fn = void(__thiscall*)(void*, PaintMode_t);
	static auto original = m_vguihook.Get<Fn>(Interfaces::vgui->GetOffset(Off_Paint));
	return original(Interfaces::vgui->Inst(), mode);
}

void __stdcall CPaintHook::Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	const auto& res = OnPaintTraverse.DispatchEvent(vguiPanel, forceRepaint, allowForce);
	if (res.Flags & EventReturnFlags::NoOriginal)
		return;

	g_hk_panel.PaintTraverse(vguiPanel, forceRepaint, allowForce);
}

void __stdcall CPaintHook::Hooked_Paint(UNCRAP PaintMode_t mode)
{
	g_hk_panel.Paint(mode);
	OnPaint.DispatchEvent(mode);
}
