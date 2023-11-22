#include "OverlayHook.h"
#include <Base/Sig.h>
#include <Base/AsmTools.h>

#ifdef _WIN64
#define OVERLAY_MODULE "GameOverlayRenderer64.dll"
#define OVERLAY_RESET_SIG "48 8B CF FF 15 ? ? ? ? 8B F8 "
#define OVERLAY_RESET_OFF 5
#define OVERLAY_PRESENT_SIG "FF 15 ? ? ? ? 8B F8 EB 1E"
#define OVERLAY_PRESENT_OFF 2
#else
#define OVERLAY_MODULE "GameOverlayRenderer.dll"
#define OVERLAY_RESET_SIG "68 ? ? ? ? 68 ? ? ? ? FF 76 40 E8 ? ? ? ? 83 C4 10 68"
#define OVERLAY_RESET_OFF 1
#define OVERLAY_PRESENT_SIG "68 ? ? ? ? 68 ? ? ? ? FF 76 44"
#define OVERLAY_PRESENT_OFF 1
#endif

COverlayHook::COverlayHook() : m_dev(nullptr), BASEHOOK(COverlayHook)
{
	RegisterEvent(EVENT_DX9PRESENT);
	RegisterEvent(EVENT_DX9RESET);
}

void COverlayHook::Hook()
{
	UINT_PTR presentcall = Sig::FindPattern(OVERLAY_MODULE, OVERLAY_PRESENT_SIG);
	UINT_PTR resetcall = Sig::FindPattern(OVERLAY_MODULE, OVERLAY_RESET_SIG);

	if (!presentcall || !resetcall)
		FATAL("Failed to find signatures in " OVERLAY_MODULE);

	m_pPresent = AsmTools::Relative<void**>(presentcall, OVERLAY_PRESENT_OFF);
	m_pReset = AsmTools::Relative<void**>(resetcall, OVERLAY_RESET_OFF);

	m_oldpresent = *m_pPresent;
	m_oldreset = *m_pReset;

	*m_pPresent = &Hooked_Present;
	*m_pReset = &Hooked_Reset;
}

void COverlayHook::Unhook()
{
	*m_pPresent = m_oldpresent;
	*m_pReset = m_oldreset;
}

HRESULT WINAPI COverlayHook::Hooked_Reset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* Params)
{
	g_hk_overlay.Device() = thisptr;
	g_hk_overlay.PushEvent(EVENT_DX9RESET);
	return g_hk_overlay.Reset()(thisptr, Params);
}

HRESULT WINAPI COverlayHook::Hooked_Present(IDirect3DDevice9* thisptr, const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion)
{
	g_hk_overlay.Device() = thisptr;

	DWORD oldstate;
	thisptr->GetRenderState(D3DRS_SRGBWRITEENABLE, &oldstate);
	thisptr->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	g_hk_overlay.PushEvent(EVENT_DX9PRESENT);

	thisptr->SetRenderState(D3DRS_SRGBWRITEENABLE, oldstate);
	return g_hk_overlay.Present()(thisptr, Src, Dest, Window, DirtyRegion);
}
