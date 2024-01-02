#pragma once
#include <Hooks/Hooks.h>
#include <cassert>
#include <SDK/platform.h>
#include <SDK/commonmacros.h>
#include <SDK/ishaderapi.h>

DECL_EVENT(EVENT_READ_PIXELS);

class ShaderApiHook : public CBaseHook
{
public:
    ShaderApiHook();
    virtual void Hook() override;
    virtual void Unhook() override;
    inline IShaderAPI* ShaderApi() const { return m_shaderapi; }

    void ReadPixels(Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *pData, ImageFormat dstFormat, int nDstStride);
    void ReadPixels(int x, int y, int width, int height, unsigned char *pData, ImageFormat dstFormat);

private:
    CVMTHook m_vmt;
    IShaderAPI* m_shaderapi;

    static void __stdcall Hooked_ReadPixels_0(UNCRAP Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *pData, ImageFormat dstFormat, int nDstStride);
    static void __stdcall Hooked_ReadPixels_1(UNCRAP int x, int y, int width, int height, unsigned char *pData, ImageFormat dstFormat);
};

inline ShaderApiHook g_hk_shaderapi;