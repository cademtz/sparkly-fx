#include "d3d9.h"

namespace Helper
{

bool GetD3DFormatInfo(D3DFORMAT fmt, D3DFORMAT_info* out_info)
{
    switch (fmt)
    {
    //                                                                              |is_depth
    //                                           |bitdepth        |is_uniform_bitdepth
    //                                        |stride          |num_channels|is_float
    case D3DFMT_R8G8B8:         *out_info = { 3, {8,8,8},      3, true,     false,  false }; break;
    case D3DFMT_A8R8G8B8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_X8R8G8B8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_R5G6B5:         *out_info = { 2, {5,6,5},      3, false,    false,  false }; break;
    case D3DFMT_X1R5G5B5:       *out_info = { 2, {1,5,5,5},    4, false,    false,  false }; break;
    case D3DFMT_A1R5G5B5:       *out_info = { 2, {1,5,5,5},    4, false,    false,  false }; break;
    case D3DFMT_A4R4G4B4:       *out_info = { 2, {4,4,4,4},    4, true,     false,  false }; break;
    case D3DFMT_R3G3B2:         *out_info = { 1, {3,3,2},      3, false,    false,  false }; break;
    case D3DFMT_A8:             *out_info = { 1, {8},          1, true,     false,  false }; break;
    case D3DFMT_A8R3G3B2:       *out_info = { 2, {8,3,3,2},    4, false,    false,  false }; break;
    case D3DFMT_X4R4G4B4:       *out_info = { 2, {4,4,4,4},    4, true,     false,  false }; break;
    case D3DFMT_A2B10G10R10:    *out_info = { 4, {2,10,10,10}, 4, false,    false,  false }; break;
    case D3DFMT_A8B8G8R8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_X8B8G8R8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_G16R16:         *out_info = { 4, {16,16},      2, true,     false,  false }; break;
    case D3DFMT_A2R10G10B10:    *out_info = { 4, {2,10,10,10}, 4, false,    false,  false }; break;
    case D3DFMT_A16B16G16R16:   *out_info = { 8, {16,16,16,16},4, true,     false,  false }; break;
    case D3DFMT_A8P8:           *out_info = { 2, {8,8},        2, true,     false,  false }; break;
    case D3DFMT_P8:             *out_info = { 1, {8},          1, true,     false,  false }; break;
    case D3DFMT_L8:             *out_info = { 1, {8},          1, true,     false,  false }; break;
    case D3DFMT_A8L8:           *out_info = { 2, {8,8},        2, true,     false,  false }; break;
    case D3DFMT_A4L4:           *out_info = { 1, {4,4},        2, true,     false,  false }; break;
    case D3DFMT_V8U8:           *out_info = { 2, {8,8},        2, true,     false,  false }; break;
    case D3DFMT_L6V5U5:         *out_info = { 2, {6,5,5},      3, false,    false,  false }; break;
    case D3DFMT_X8L8V8U8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_Q8W8V8U8:       *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;
    case D3DFMT_V16U16:         *out_info = { 4, {8,8,8,8},    2, true,     false,  false }; break;
    case D3DFMT_A2W10V10U10:    *out_info = { 4, {2,10,10,10}, 4, false,    false,  false }; break;
    //case D3DFMT_UYVY:
    //case D3DFMT_R8G8_B8G8:
    //case D3DFMT_YUY2:
    //case D3DFMT_G8R8_G8B8:
    //case D3DFMT_DXT1:
    //case D3DFMT_DXT2:
    //case D3DFMT_DXT3:
    //case D3DFMT_DXT4:
    //case D3DFMT_DXT5:
    case FOURCC_INTZ:           *out_info = { 4, {24,8},       2, false,    false,  true  }; break;
    case D3DFMT_D16_LOCKABLE:   *out_info = { 2, {16},         1, true,     false,  true  }; break;
    case D3DFMT_D32:            *out_info = { 4, {32},         1, true,     false,  true  }; break;
    case D3DFMT_D15S1:          *out_info = { 2, {15,1},       2, false,    false,  true  }; break;
    case D3DFMT_D24S8:          *out_info = { 4, {24,8},       2, false,    false,  true  }; break;
    case D3DFMT_D24X8:          *out_info = { 4, {24,8},       2, false,    false,  true  }; break;
    case D3DFMT_D24X4S4:        *out_info = { 4, {24,4,4},     3, false,    false,  true  }; break;
    case D3DFMT_D16:            *out_info = { 2, {16},         1, true,     false,  true  }; break;
    case D3DFMT_D32F_LOCKABLE:  *out_info = { 4, {32},         1, true,     true,   true  }; break;
    case D3DFMT_D24FS8:         *out_info = { 4, {24,8},       2, false,    true,   true  }; break;
    /* Z-Stencil formats valid for CPU access */
    case D3DFMT_D32_LOCKABLE:   *out_info = { 4, {32},         1, true,     false,  true  }; break;
    case D3DFMT_S8_LOCKABLE:    *out_info = { 1, {8},          1, true,     false,  true  }; break;
    /* -- D3D9Ex only */
    case D3DFMT_L16:            *out_info = { 2, {16},         1, true,     false,  false }; break;
    //case D3DFMT_VERTEXDATA:
    case D3DFMT_INDEX16:        *out_info = { 2, {16},         1, true,     false,  false }; break;
    case D3DFMT_INDEX32:        *out_info = { 4, {32},         1, true,     false,  false }; break;
    case D3DFMT_Q16W16V16U16:   *out_info = { 8, {16,16,16,16},4, true,     false,  false }; break;
    case D3DFMT_MULTI2_ARGB8:   *out_info = { 4, {8,8,8,8},    4, true,     false,  false }; break;

    // Floating point surface formats

    // s10e5 formats (16-bits per channel)
    case D3DFMT_R16F:           *out_info = { 2, {16},         1, true,     true,   false }; break;
    case D3DFMT_G16R16F:        *out_info = { 4, {16,16},      2, true,     true,   false }; break;
    case D3DFMT_A16B16G16R16F:  *out_info = { 8, {16,16,16,16},4, true,     true,   false }; break;
    // IEEE s23e8 formats (32-bits per channel)
    case D3DFMT_R32F:           *out_info = { 4, {32},         1, true,     true,   false }; break;
    case D3DFMT_G32R32F:        *out_info = { 8, {32,32},      2, true,     true,   false }; break;
    case D3DFMT_A32B32G32R32F:  *out_info = { 16,{32,32,32,32},4, true,     true,   false }; break;
    //case D3DFMT_CxV8U8:

    /* D3D9Ex only -- */
    // Monochrome 1 bit per pixel format
    case D3DFMT_A1:             *out_info = { 1, {1},          1, true,     false,  false }; break;
    // 2.8 biased fixed point:
    //case D3DFMT_A2B10G10R10_XR_BIAS:
    // Binary format indicating that the data has no inherent type
    //case D3DFMT_BINARYBUFFER:
    case D3DFMT_B8G8R8:         *out_info = { 3, {8,8,8},      3, true,     false,  false }; break;
    default:
        return false;
    }

    return true;
}

const char* GetD3DFormatAsFFmpegPixFmt(D3DFORMAT fmt, bool skip_alpha)
{
    switch (fmt)
    {
    case D3DFMT_R8G8B8:         return "bgr24";

    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
        return skip_alpha ? "bgr0" : "bgra";

    case D3DFMT_R5G6B5:         return "bgr565le";
    case D3DFMT_X1R5G5B5:       return nullptr;
    case D3DFMT_A1R5G5B5:       return nullptr;
    case D3DFMT_A4R4G4B4:       return nullptr;
    case D3DFMT_R3G3B2:         return "bgr8";
    case D3DFMT_A8:             return "gray";
    case D3DFMT_A8R3G3B2:       return nullptr;
    case D3DFMT_X4R4G4B4:       return nullptr;
    case D3DFMT_A2B10G10R10:    return nullptr;

    case D3DFMT_A8B8G8R8:
    case D3DFMT_X8B8G8R8:
        return skip_alpha ? "rgb0" : "rgba";

    case D3DFMT_G16R16:         return nullptr;
    case D3DFMT_A2R10G10B10:    return nullptr;
    case D3DFMT_A16B16G16R16:   return skip_alpha ? nullptr : "rgba64le";
    case D3DFMT_A8P8:           return nullptr;
    case D3DFMT_P8:             return nullptr;
    case D3DFMT_L8:             return nullptr;
    case D3DFMT_A8L8:           return nullptr;
    case D3DFMT_A4L4:           return nullptr;
    case D3DFMT_V8U8:           return nullptr;
    case D3DFMT_L6V5U5:         return nullptr;
    case D3DFMT_X8L8V8U8:       return nullptr;
    case D3DFMT_Q8W8V8U8:       return nullptr;
    case D3DFMT_V16U16:         return nullptr;
    case D3DFMT_A2W10V10U10:    return nullptr;
    case D3DFMT_UYVY:           return "uyvy422";
    //case D3DFMT_R8G8_B8G8:
    case D3DFMT_YUY2:           return "yuyv422";
    //case D3DFMT_G8R8_G8B8:
    //case D3DFMT_DXT1:
    //case D3DFMT_DXT2:
    //case D3DFMT_DXT3:
    //case D3DFMT_DXT4:
    //case D3DFMT_DXT5:
    case FOURCC_INTZ:           return nullptr;
    case D3DFMT_D16_LOCKABLE:   return "gray16le";
    case D3DFMT_D32:            return nullptr;
    case D3DFMT_D15S1:          return nullptr;
    case D3DFMT_D24S8:          return nullptr;
    case D3DFMT_D24X8:          return nullptr;
    case D3DFMT_D24X4S4:        return nullptr;
    case D3DFMT_D16:            return nullptr;
    case D3DFMT_D32F_LOCKABLE:  return "grayf32le";
    case D3DFMT_D24FS8:         return nullptr;
    case D3DFMT_L16:            return nullptr;
    case D3DFMT_INDEX16:        return nullptr;
    case D3DFMT_INDEX32:        return nullptr;
    case D3DFMT_Q16W16V16U16:   return nullptr;
    case D3DFMT_MULTI2_ARGB8:   return nullptr;
    case D3DFMT_R16F:           return nullptr;
    case D3DFMT_G16R16F:        return nullptr;
    case D3DFMT_A16B16G16R16F:  return skip_alpha ? nullptr : "rgbaf16le";
    case D3DFMT_R32F:           return "grayf32le";
    case D3DFMT_G32R32F:        return nullptr;
    case D3DFMT_A32B32G32R32F:  return skip_alpha ? nullptr : "rgbaf32le";
    case D3DFMT_A1:             return "monob";
    default:
        return nullptr;
    };
}

void Invert4x4Matrix(const float *m, float *out)
{

/* NB. OpenGL Matrices are COLUMN major. */
//#define MAT(m,r,c) (m)[(c)*4+(r)]

// Cade: DirectX matrices are ROW major.
#define MAT(m,r,c) (m)[(r)*4+(c)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

    static const float _identity[] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    float det;
    float d12, d13, d23, d24, d34, d41;
    float tmp[16]; /* Allow out == in. */

    /* Inverse = adjoint / det. (See linear algebra texts.)*/

    /* pre-compute 2x2 dets for last two rows when computing */
    /* cofactors of first two rows. */
    d12 = (m31*m42-m41*m32);
    d13 = (m31*m43-m41*m33);
    d23 = (m32*m43-m42*m33);
    d24 = (m32*m44-m42*m34);
    d34 = (m33*m44-m43*m34);
    d41 = (m34*m41-m44*m31);

    tmp[0] =  (m22 * d34 - m23 * d24 + m24 * d23);
    tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
    tmp[2] =  (m21 * d24 + m22 * d41 + m24 * d12);
    tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

    /* Compute determinant as early as possible using these cofactors. */
    det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];

    /* Run singularity test. */
    if (det == 0.0) {
        /* printf("invert_matrix: Warning: Singular matrix.\n"); */
        memcpy(out,_identity,16*sizeof(float));
    }
    else {
        float invDet = 1.0 / det;
        /* Compute rest of inverse. */
        tmp[0] *= invDet;
        tmp[1] *= invDet;
        tmp[2] *= invDet;
        tmp[3] *= invDet;
        
        tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
        tmp[5] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
        tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
        tmp[7] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;
        
        /* Pre-compute 2x2 dets for first two rows when computing */
        /* cofactors of last two rows. */
        d12 = m11*m22-m21*m12;
        d13 = m11*m23-m21*m13;
        d23 = m12*m23-m22*m13;
        d24 = m12*m24-m22*m14;
        d34 = m13*m24-m23*m14;
        d41 = m14*m21-m24*m11;
        
        tmp[8] =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
        tmp[9] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
        tmp[10] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
        tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
        tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
        tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
        tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
        tmp[15] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
        
        memcpy(out, tmp, 16*sizeof(float));
    }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}

void DrawTexturedRect(IDirect3DDevice9* device, std::array<float, 4> screen_rect, std::array<float, 4> texture_rect)
{
    // The following code is adapted from the Half-Life Advanced Effects project (advancedfx.org)

    constexpr DWORD RECT_BUFFER_FVF = (D3DFVF_XYZRHW | D3DFVF_TEX1);
    struct RectBufferVertex
    {
        float x, y, z, rhw;
        float u, v;
    };

    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
    device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    device->SetVertexShader(NULL);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS); // redundant due to ZENABLE
    device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
    device->SetDepthStencilSurface(nullptr);

    D3DMATRIX proj_matrix = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };

    device->SetTransform(D3DTS_WORLD, &proj_matrix);
    device->SetTransform(D3DTS_VIEW, &proj_matrix);
    device->SetTransform(D3DTS_PROJECTION, &proj_matrix);

    RectBufferVertex vertices[4] = {
        {screen_rect[0] + screen_rect[2]*0, screen_rect[1] + screen_rect[3]*1, 0, 0, texture_rect[0] + texture_rect[2]*0, texture_rect[1] + texture_rect[3]*1},
        {screen_rect[0] + screen_rect[2]*0, screen_rect[1] + screen_rect[3]*0, 0, 0, texture_rect[0] + texture_rect[2]*0, texture_rect[1] + texture_rect[3]*0},
        {screen_rect[0] + screen_rect[2]*1, screen_rect[1] + screen_rect[3]*1, 0, 0, texture_rect[0] + texture_rect[2]*1, texture_rect[1] + texture_rect[3]*1},
        {screen_rect[0] + screen_rect[2]*1, screen_rect[1] + screen_rect[3]*0, 0, 0, texture_rect[0] + texture_rect[2]*1, texture_rect[1] + texture_rect[3]*0},
    };
    device->SetFVF(RECT_BUFFER_FVF);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &vertices, sizeof(vertices[0]));
}

}