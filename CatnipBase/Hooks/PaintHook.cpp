#include "PaintHook.h"
#include "Base/Interfaces.h"

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
	static auto hook = GETHOOK(CPaintHook);
	auto ctx = hook->Context();
	ctx->panel = vguiPanel, ctx->forceRepaint = forceRepaint, ctx->allowForce = allowForce;

	int flags = hook->PushEvent(EVENT_PAINTTRAVERSE);
	if (flags & Return_NoOriginal)
		return;

	hook->PaintTraverse(vguiPanel, forceRepaint, allowForce);
}

void __stdcall CPaintHook::Hooked_Paint(UNCRAP PaintMode_t mode)
{
	static auto hook = GETHOOK(CPaintHook);
	hook->Context()->mode = mode;
	hook->Paint(mode);
	hook->PushEvent(EVENT_PAINT);
}
