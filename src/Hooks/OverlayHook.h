#pragma once
#include "Hooks.h"
#include <d3d9.h>

DECL_EVENT(EVENT_DX9PRESENT);
DECL_EVENT(EVENT_DX9RESET);

class COverlayHook : public CBaseHook
{
public:
	COverlayHook();

	void Hook() override;
	void Unhook() override;

	inline IDirect3DDevice9* Device() { return m_dev; }
	HRESULT Reset(D3DPRESENT_PARAMETERS* Params);
	HRESULT Present(const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion);

private:
	IDirect3DDevice9* m_dev;
	CJumpHook m_jmp_reset;
	CJumpHook m_jmp_present;

	static HRESULT WINAPI Hooked_Reset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* Params);
	static HRESULT WINAPI Hooked_Present(
		IDirect3DDevice9* thisptr, const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion
	);
};

inline COverlayHook g_hk_overlay;