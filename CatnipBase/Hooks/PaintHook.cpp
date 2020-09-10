#include "PaintHook.h"
#include "Base/Interfaces.h"

#define OFF_PAINTTRAVERSE 41

CPaintHook::CPaintHook() : BASEHOOK(CPaintHook)
{
	RegisterEvent(EVENT_PAINTTRAVERSE);
	RegisterEvent(EVENT_PAINT);
}

void CPaintHook::Hook()
{
	//m_hook.Hook(Interfaces::panels);
	m_hook.Hook(Interfaces::vgui);
	m_hook.Set(13, &Hooked_Paint);
	//m_hook.Set(OFF_PAINTTRAVERSE, Hooked_PaintTraverse);
}

void CPaintHook::Unhook() {
	m_hook.Unhook();
}

void CPaintHook::PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	using Fn = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);
	static auto original = m_hook.Get<Fn>(OFF_PAINTTRAVERSE);
	return original(Interfaces::panels, vguiPanel, forceRepaint, allowForce);
}

void CPaintHook::Paint(PaintMode_t mode)
{
	using Fn = void(__thiscall*)(void*, PaintMode_t);
	static auto original = m_hook.Get<Fn>(13);
	return original(Interfaces::vgui, mode);
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
	hook->PushEvent(EVENT_PAINT);
	hook->Paint(mode);
}
