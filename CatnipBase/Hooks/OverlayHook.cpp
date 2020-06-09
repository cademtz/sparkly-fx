#include "OverlayHook.h"
#include "Base/Sig.h"

#ifdef _WIN64
#define OVERLAY_MODULE "GameOverlayRenderer64.dll"
#define OVERLAY_RESET_SIG "FF 15 ?? ?? ?? ?? 8B F8 85 C0 78 23"
#define OVERLAY_PRESENT_SIG "8D 05 ?? ?? ?? ?? 48 8D 15 92 C6 FF FF"
#else
#define OVERLAY_MODULE "GameOverlayRenderer.dll"
#define OVERLAY_RESET_SIG "FF 15 ? ? ? ? 8B F8 85 FF 78 18"
#define OVERLAY_PRESENT_SIG "FF 15 ?? ?? ?? ?? 8B F8 85 DB"
#endif

COverlayHook::COverlayHook() : BASEHOOK(COverlayHook)
{
	UINT_PTR presentcall = Sig::FindPattern(OVERLAY_MODULE, OVERLAY_PRESENT_SIG);
	UINT_PTR resetcall = Sig::FindPattern(OVERLAY_MODULE, OVERLAY_RESET_SIG);

	if (!presentcall || !resetcall)
		FATAL("Failed to find signatures in " OVERLAY_MODULE);

	DWORD present_off = *(DWORD*)(presentcall + 2);
	DWORD reset_off = *(DWORD*)(resetcall + 2);

#ifdef _WIN64
	m_pPresent = (void**)(presentcall + present_off + 6);
	m_pReset = (void**)(resetcall + reset_off + 6);
#else
	m_pPresent = (void**)present_off;
	m_pReset = (void**)reset_off;
#endif

	m_oldpresent = *m_pPresent;
	m_oldreset = *m_pReset;

	*m_pPresent = &Hooked_Present;
	*m_pReset = &Hooked_Reset;

	RegisterEvent(EVENT_DX9PRESENT);
	RegisterEvent(EVENT_DX9RESET);
}

void COverlayHook::Unhook()
{
	*m_pPresent = m_oldpresent;
	*m_pReset = m_oldreset;
}

HRESULT WINAPI COverlayHook::Hooked_Reset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* Params)
{
	auto hook = GETHOOK(COverlayHook);
	hook->Device() = thisptr;
	hook->PushEvent(EVENT_DX9RESET, 0);
	return hook->Reset()(thisptr, Params);
}

HRESULT WINAPI COverlayHook::Hooked_Present(IDirect3DDevice9* thisptr, const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion)
{
	auto hook = GETHOOK(COverlayHook);
	hook->Device() = thisptr;
	hook->PushEvent(EVENT_DX9PRESENT, 0);
	return hook->Present()(thisptr, Src, Dest, Window, DirtyRegion);
}
