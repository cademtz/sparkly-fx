#pragma once
#include "Hooks.h"
#include <d3d9.h>
#include <unordered_map>
#include <string>

class COverlayHook : public CBaseHook
{
public:
	COverlayHook() : BASEHOOK(COverlayHook), m_dev{nullptr} {}

	void Hook() override;
	void Unhook() override;

	IDirect3DDevice9* Device() const { return m_dev; }
	HRESULT Reset(D3DPRESENT_PARAMETERS* Params);
	HRESULT Present(const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion);

	using ResetEvent = EventSource<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;
	using PresentEvent = EventSource<void(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)>;

	static inline ResetEvent OnReset;
	static inline PresentEvent OnPresent;

	// === Sparkly FX code === //

	void ReplaceDepthStencil(IDirect3DSurface9* old_stencil, IDirect3DSurface9* new_stencil);
	void RestoreDepthStencil(IDirect3DSurface9* old_stencil);
	HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
	void ReplaceRenderTarget(IDirect3DSurface9* old_target, IDirect3DSurface9* new_target);
	void RestoreRenderTarget(IDirect3DSurface9* old_target);
	HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);

private:
	IDirect3DDevice9* m_dev;
	CJumpHook m_jmp_reset;
	CJumpHook m_jmp_present;

	static HRESULT WINAPI Hooked_Reset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* Params);
	static HRESULT WINAPI Hooked_Present(
		IDirect3DDevice9* thisptr, const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion
	);

	// === Sparkly FX code === //

	CJumpHook m_jmp_setstencil;
	CJumpHook m_jmp_setrendertarget;
	std::unordered_map<IDirect3DSurface9*, IDirect3DSurface9*> m_stencilmap;
	std::unordered_map<IDirect3DSurface9*, IDirect3DSurface9*> m_rendertarget_map;

	static HRESULT WINAPI Hooked_SetDepthStencilSurface(IDirect3DDevice9* thisptr, IDirect3DSurface9* pNewZStencil);
	static HRESULT WINAPI Hooked_SetRenderTarget(IDirect3DDevice9* thisptr, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	/**
	 * @brief Get the device vtable by signature scanning
	 * @param out_error A string to store the error message
	 * @return `nullptr` on failure
	 */
	static void** GetDeviceVtable_SigScan(std::string* out_error);
	/**
	 * @brief Get the device vtable by creating a temporary device.
	 * 
	 * This will exit the program on failure. Use only as a last-resort.
	 * @param out_error A string to store the error message
	 */
	static void** GetDeviceVtable_CreateDevice(std::string* out_error);
};

inline COverlayHook g_hk_overlay;