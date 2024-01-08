#include "OverlayHook.h"
#include <Base/Sig.h>
#include <Base/AsmTools.h>
#include <cstdio>

#ifdef WIN64
#define SIG_CD3DHAL "48 8D 05 ? ? ? ? 48 89 03 33 C0"
#define OFFSET_CD3DHAL 3
#else
#define SIG_CD3DHAL "C7 06 ? ? ? ? 89 86 ? ? ? ? 89 86"
#define OFFSET_CD3DHAL 2
#endif

COverlayHook::COverlayHook() : m_dev(nullptr), BASEHOOK(COverlayHook)
{
	RegisterEvent(EVENT_DX9PRESENT);
	RegisterEvent(EVENT_DX9RESET);
}

void COverlayHook::Hook()
{
	std::string error_create;
	std::string error_scan;

	printf("Creating IDirect3DDevice9...\n");
	void** vtable = GetDeviceVtable_CreateDevice(&error_create);
	
	if (!vtable)
	{
		printf("Scanning for IDirect3DDevice9 vtable...\n");
		vtable = GetDeviceVtable_SigScan(&error_scan);
	}

	if (!vtable)
	{
		FATAL(
			"Failed to hook D3D9.\n"
			"GetDeviceVtable_CreateDevice: %s\n"
			"GetDeviceVtable_SigScan: %s",
			error_create.c_str(), error_scan.c_str()
		);
	}
	
	m_jmp_reset.Hook(vtable[16], &Hooked_Reset);
	m_jmp_present.Hook(vtable[17], &Hooked_Present);
	m_jmp_setstencil.Hook(vtable[39], &Hooked_SetDepthStencilSurface);
	m_jmp_setrendertarget.Hook(vtable[37], &Hooked_SetRenderTarget);
}

void COverlayHook::Unhook()
{
	m_jmp_reset.Unhook();
	m_jmp_present.Unhook();
	m_jmp_setstencil.Unhook();
	m_jmp_setrendertarget.Unhook();
}

void COverlayHook::ReplaceDepthStencil(IDirect3DSurface9* old_stencil, IDirect3DSurface9* new_stencil) {
	m_stencilmap[old_stencil] = new_stencil;
}
void COverlayHook::RestoreDepthStencil(IDirect3DSurface9* old_stencil) {
	m_stencilmap.erase(old_stencil);
}
void COverlayHook::ReplaceRenderTarget(IDirect3DSurface9* old_target, IDirect3DSurface9* new_target) {
	m_rendertarget_map[old_target] = new_target;
}
void COverlayHook::RestoreRenderTarget(IDirect3DSurface9* old_target) {
	m_rendertarget_map.erase(old_target);
}

HRESULT COverlayHook::Reset(D3DPRESENT_PARAMETERS* Params)
{
	static auto original = m_jmp_reset.Original<decltype(Hooked_Reset)*>();
	return original(m_dev, Params);
}

HRESULT COverlayHook::Present(const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion)
{
	static auto original = m_jmp_present.Original<decltype(Hooked_Present)*>();
	return original(m_dev, Src, Dest, Window, DirtyRegion);
}

HRESULT COverlayHook::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	static auto original = m_jmp_setstencil.Original<decltype(Hooked_SetDepthStencilSurface)*>();
	return original(m_dev, pNewZStencil);
}

HRESULT WINAPI COverlayHook::Hooked_SetDepthStencilSurface(IDirect3DDevice9* thisptr, IDirect3DSurface9* pNewZStencil)
{
	g_hk_overlay.m_dev = thisptr;
	auto it = g_hk_overlay.m_stencilmap.find(pNewZStencil);
	if (it != g_hk_overlay.m_stencilmap.end())
		pNewZStencil = it->second;
	return g_hk_overlay.SetDepthStencilSurface(pNewZStencil);
}

HRESULT WINAPI COverlayHook::Hooked_SetRenderTarget(IDirect3DDevice9* thisptr, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	g_hk_overlay.m_dev = thisptr;
	auto it = g_hk_overlay.m_rendertarget_map.find(pRenderTarget);
	if (it != g_hk_overlay.m_rendertarget_map.end())
		pRenderTarget = it->second;
	return g_hk_overlay.SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT COverlayHook::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	static auto original = m_jmp_setrendertarget.Original<decltype(Hooked_SetRenderTarget)*>();
	return original(m_dev, RenderTargetIndex, pRenderTarget);
}

HRESULT WINAPI COverlayHook::Hooked_Reset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* Params)
{
	g_hk_overlay.m_dev = thisptr;
	g_hk_overlay.PushEvent(EVENT_DX9RESET);
	return g_hk_overlay.Reset(Params);
}

HRESULT WINAPI COverlayHook::Hooked_Present(IDirect3DDevice9* thisptr, const RECT* Src, const RECT* Dest, HWND Window, const RGNDATA* DirtyRegion)
{
	g_hk_overlay.m_dev = thisptr;

	DWORD oldstate;
	thisptr->GetRenderState(D3DRS_SRGBWRITEENABLE, &oldstate);
	thisptr->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	g_hk_overlay.PushEvent(EVENT_DX9PRESENT);

	thisptr->SetRenderState(D3DRS_SRGBWRITEENABLE, oldstate);
	return g_hk_overlay.Present(Src, Dest, Window, DirtyRegion);
}

void** COverlayHook::GetDeviceVtable_SigScan(std::string* out_error)
{
	uint8_t* code = (uint8_t*)Sig::FindPattern("d3d9.dll", SIG_CD3DHAL);
	if (!code)
	{
		*out_error = "The signature scan could not find `CD3DHal::CD3DHal`";
		return nullptr;
	}
	
	
	return (void**)AsmTools::Relative(code, OFFSET_CD3DHAL);
}

void** COverlayHook::GetDeviceVtable_CreateDevice(std::string* out_error)
{
	HMODULE d3d_module = Base::GetModule("d3d9.dll");
	if (!d3d_module)
	{
		*out_error = "d3d9.dll is not loaded";
		return nullptr;
	}
	
	auto p_Direct3DCreate9 = (decltype(Direct3DCreate9)*)Base::GetProc(d3d_module, "Direct3DCreate9");

	IDirect3D9* d3d = p_Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
	{
		*out_error = "Direct3DCreate9 failed";
		return nullptr;
	}
	
	D3DDISPLAYMODE display_mode;
	if (FAILED(d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode)))
	{
		*out_error = "GetAdapterDisplayMode failed";
		return nullptr;
	}
	
	D3DPRESENT_PARAMETERS present_params = {0};
	present_params.Windowed = TRUE;
	present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	present_params.BackBufferFormat = display_mode.Format;

	WNDCLASSEX wnd_class = {0};
	wnd_class.cbSize = sizeof(wnd_class);
	wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	wnd_class.lpfnWndProc = DefWindowProc;
	wnd_class.lpszClassName = TEXT("DummyWindowClass");
	wnd_class.hInstance = GetModuleHandle(NULL);
	
	if (!RegisterClassEx(&wnd_class))
	{
		*out_error = "Failed to register temp window class";
		return nullptr;
	}

	HWND temp_window = CreateWindow(
		wnd_class.lpszClassName, TEXT("Dummy window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wnd_class.hInstance, NULL
	);
	if (!temp_window)
	{
		*out_error = "Failed to create temp window";
		UnregisterClass(wnd_class.lpszClassName, NULL);
		return nullptr;
	}

	IDirect3DDevice9* d3d_device;
	HRESULT err = d3d->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, temp_window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&present_params, &d3d_device
	);

	// Device creation can fail when `present_params.Windowed` does not match the game's windowed state.
	// Let's try again with the opposite value.
	if (FAILED(err))
	{
		present_params.Windowed = !present_params.Windowed;
		err = d3d->CreateDevice(
			D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, temp_window,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&present_params, &d3d_device
		);
	}

	DestroyWindow(temp_window);
	UnregisterClass(wnd_class.lpszClassName, NULL);

	void** vtable = nullptr;

	if (SUCCEEDED(err))
		vtable = *(void***)d3d_device;
	else
	{
		*out_error = "CreateDevice error code: " + std::to_string(err);
		return nullptr;
	}

	// Intentionally leak the D3D device...
	// During game startup, releasing it can cause the D3D code to be unloaded or moved.
	// MinHook will throw MH_ERROR_NOT_EXECUTABLE:
	//   "The specified pointer is invalid. It points the address of non-allocated
	//   and/or non-executable region."
	
	//d3d_device->Release();
	//d3d->Release();
	return vtable;
}