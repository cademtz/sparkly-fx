#pragma once
#include "Hooks.h"
#include <SDK/VGUI.h>
#include <SDK/ienginevgui.h>

DECL_EVENT(EVENT_PAINTTRAVERSE);
DECL_EVENT(EVENT_PAINT);

struct PaintHook_Ctx
{
	vgui::VPANEL panel;
	bool forceRepaint, allowForce;
	PaintMode_t mode;
};

class CPaintHook : public CBaseHook
{
public:
	CPaintHook();

	inline PaintHook_Ctx* Context() { return &m_ctx; }

	void Hook() override;
	void Unhook() override;
	void PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
	void Paint(PaintMode_t mode);

private:
	CVMTHook m_vguihook;
	PaintHook_Ctx m_ctx;

	static void __stdcall Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true);
	static void __stdcall Hooked_Paint(UNCRAP PaintMode_t mode);
};

inline CPaintHook g_hk_panel;