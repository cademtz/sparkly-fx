#include "PanelHook.h"
#include "Base/Interfaces.h"

#define OFF_PAINTTRAVERSE 41

CPanelHook::CPanelHook() : BASEHOOK(CPanelHook) {
	RegisterEvent(EVENT_PAINTTRAVERSE);
}

void CPanelHook::Hook()
{
	m_hook.Hook(Interfaces::panels);
	m_hook.Set(OFF_PAINTTRAVERSE, Hooked_PaintTraverse);
}

void CPanelHook::Unhook() {
	m_hook.Unhook();
}

void CPanelHook::PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	using Fn = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);
	static auto original = m_hook.Get<Fn>(OFF_PAINTTRAVERSE);
	return original(Interfaces::panels, vguiPanel, forceRepaint, allowForce);
}

void __stdcall CPanelHook::Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	static auto hook = GETHOOK(CPanelHook);
	auto ctx = hook->Context();
	ctx->panel = vguiPanel, ctx->forceRepaint = forceRepaint, ctx->allowForce = allowForce;

	int flags = hook->PushEvent(EVENT_PAINTTRAVERSE);
	if (flags & Return_NoOriginal)
		return;

	hook->PaintTraverse(vguiPanel, forceRepaint, allowForce);
}
