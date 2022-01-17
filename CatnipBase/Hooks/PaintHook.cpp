#include "PaintHook.h"
#include <Base/Interfaces.h>

// We are currently avoiding a paint traverse hook because x64 GMod sux

#define OFF_PAINTTRAVERSE 41

CPaintHook::CPaintHook() : BASEHOOK(CPaintHook)
{
	RegisterEvent(EVENT_PAINTTRAVERSE);
	RegisterEvent(EVENT_PAINT);
}

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
	auto ctx = g_hk_panel.Context();
	ctx->panel = vguiPanel, ctx->forceRepaint = forceRepaint, ctx->allowForce = allowForce;

	int flags = g_hk_panel.PushEvent(EVENT_PAINTTRAVERSE);
	if (flags & Return_NoOriginal)
		return;

	g_hk_panel.PaintTraverse(vguiPanel, forceRepaint, allowForce);
}

void __stdcall CPaintHook::Hooked_Paint(UNCRAP PaintMode_t mode)
{
	g_hk_panel.Context()->mode = mode;
	g_hk_panel.Paint(mode);
	g_hk_panel.PushEvent(EVENT_PAINT);
}
