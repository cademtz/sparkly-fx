#pragma once
#include "Hooks.h"
#include "SDK/VGUI.h"
#include "SDK/ienginevgui.h"

DECL_EVENT(EVENT_PAINTTRAVERSE);
DECL_EVENT(EVENT_PAINT);

struct PanelHook_Ctx
{
	vgui::VPANEL panel;
	bool forceRepaint, allowForce;
};

class CPaintHook : public CBaseHook
{
public:
	CPaintHook();

	inline PanelHook_Ctx* Context() { return &m_ctx; }

	void Hook() override;
	void Unhook() override;
	void PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
	void Paint(PaintMode_t mode);

private:
	CVMTHook m_hook;
	PanelHook_Ctx m_ctx;

	static void __stdcall Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true);
	static void __stdcall Hooked_Paint(UNCRAP PaintMode_t mode);
};

inline CPaintHook _g_panelhook;