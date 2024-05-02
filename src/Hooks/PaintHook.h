#pragma once
#include "Hooks.h"
#include <SDK/VGUI.h>
#include <SDK/ienginevgui.h>

class CPaintHook : public CBaseHook
{
public:
	CPaintHook() : BASEHOOK(CPaintHook) {}

	void Hook() override;
	void Unhook() override;
	void PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
	void Paint(PaintMode_t mode);

	using PaintTraverseEvent = EventSource<void(vgui::VPANEL, bool, bool)>;
	using PaintEvent = EventSource<void(PaintMode_t)>;

	static inline PaintTraverseEvent OnPaintTraverse;
	static inline PaintEvent OnPaint;
private:
	CVMTHook m_vguihook;

	static void __stdcall Hooked_PaintTraverse(UNCRAP vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true);
	static void __stdcall Hooked_Paint(UNCRAP PaintMode_t mode);
};

inline CPaintHook g_hk_panel;