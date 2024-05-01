#include "ShaderApiHook.h"
#include <Base/Base.h>
#include <SDK/interface.h>

#define INDEX_READ_PIXELS_0 157
#define INDEX_READ_PIXELS_1 158

ShaderApiHook::ShaderApiHook() : BASEHOOK(ShaderApiHook) {}

void ShaderApiHook::Hook()
{
    HMODULE shaderapi_module = GetModuleHandleA("shaderapidx9.dll");
    if (!shaderapi_module)
        FATAL("This game does not use shaderapidx9.dll");
    
    CreateInterfaceFn create_interface = (CreateInterfaceFn)GetProcAddress(shaderapi_module, "CreateInterface");
    assert(create_interface);
    
    m_shaderapi = (IShaderAPI*)create_interface(SHADERAPI_INTERFACE_VERSION, nullptr);
    if (!m_shaderapi)
        FATAL("This game does not use " SHADERAPI_INTERFACE_VERSION);

    m_vmt.Hook(m_shaderapi);
    m_vmt.Set(INDEX_READ_PIXELS_0, &Hooked_ReadPixels_0);
    m_vmt.Set(INDEX_READ_PIXELS_1, &Hooked_ReadPixels_1);
}

void ShaderApiHook::ReadPixels(Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *pData, ImageFormat dstFormat, int nDstStride)
{
    typedef void(__thiscall* prototype)(IShaderAPI*, Rect_t*, Rect_t*, unsigned char*, ImageFormat, int);
    static auto original = m_vmt.Get<prototype>(INDEX_READ_PIXELS_0);
    original(ShaderApi(), pSrcRect, pDstRect, pData, dstFormat, nDstStride);
}
void ShaderApiHook::ReadPixels(int x, int y, int width, int height, unsigned char *pData, ImageFormat dstFormat)
{
    typedef void(__thiscall* prototype)(IShaderAPI*, int, int, int, int, unsigned char*, ImageFormat);
    static auto original = m_vmt.Get<prototype>(INDEX_READ_PIXELS_1);
    original(ShaderApi(), x, y, width, height, pData, dstFormat);
}

void ShaderApiHook::Unhook() {
    m_vmt.Unhook();
}

void __stdcall ShaderApiHook::Hooked_ReadPixels_0(UNCRAP Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *pData, ImageFormat dstFormat, int nDstStride)
{
    if (ReadPixelsEvent.DispatchEvent() & EventReturnFlags::NoOriginal)
        return;
    g_hk_shaderapi.ReadPixels(pSrcRect, pDstRect, pData, dstFormat, nDstStride);
}
void __stdcall ShaderApiHook::Hooked_ReadPixels_1(UNCRAP int x, int y, int width, int height, unsigned char *pData, ImageFormat dstFormat)
{
    if (ReadPixelsEvent.DispatchEvent() & EventReturnFlags::NoOriginal)
        return;
    g_hk_shaderapi.ReadPixels(x, y, width, height, pData, dstFormat);
}
