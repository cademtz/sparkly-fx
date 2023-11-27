#include "OverlayHook.h"
#include <Base/Sig.h>
#include <Base/AsmTools.h>

COverlayHook::COverlayHook() : m_dev(nullptr), BASEHOOK(COverlayHook)
{
	RegisterEvent(EVENT_DX9PRESENT);
	RegisterEvent(EVENT_DX9RESET);
}

void COverlayHook::Hook()
{
	HMODULE d3d_module = Base::GetModule("d3d9.dll");
	auto p_Direct3DCreate9 = (decltype(Direct3DCreate9)*)Base::GetProc(d3d_module, "Direct3DCreate9");

	IDirect3D9* d3d = p_Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
		FATAL("Direct3DCreate9 failed");
	
	D3DDISPLAYMODE display_mode;
	if (FAILED(d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode)))
		FATAL("GetAdapterDisplayMode failed");
	
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
		FATAL("Failed to register temp window class");

	HWND temp_window = CreateWindow(
		wnd_class.lpszClassName, TEXT("Dummy window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wnd_class.hInstance, NULL
	);
	if (!temp_window)
		FATAL("Failed to create temp window");

	IDirect3DDevice9* d3d_device;
	HRESULT err = d3d->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, temp_window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&present_params, &d3d_device
	);

	DestroyWindow(temp_window);
	UnregisterClass(wnd_class.lpszClassName, NULL);

	if (FAILED(err))
		FATAL("Failed to create IDirect3DDevice9 object\nD3D9 Error code: %d", err);
	
	void** vtable = *(void***)d3d_device;
	m_jmp_reset.Hook(vtable[16], &Hooked_Reset);
	m_jmp_present.Hook(vtable[17], &Hooked_Present);

	d3d_device->Release();
	d3d->Release();
}

void COverlayHook::Unhook()
{
	m_jmp_reset.Unhook();
	m_jmp_present.Unhook();
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
