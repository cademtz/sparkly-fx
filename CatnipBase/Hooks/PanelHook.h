#pragma once
#include "Hooks.h"
#include "SDK/VGUI.h"

DECL_EVENT(EVENT_PAINTTRAVERSE);

struct PanelHook_Ctx
{
	vgui::VPANEL panel;
	bool forceRepaint, allowForce;
};

class CPanelHook : public CBaseHook
{
public:
	CPanelHook();

	inline PanelHook_Ctx* Context() { return &m_ctx; }

	void Hook() override;
	void Unhook() override;
	void PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);

private:
	CVMTHook m_hook;
	PanelHook_Ctx m_ctx;

	static void __stdcall Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true);
};

inline CPanelHook _g_panelhook;