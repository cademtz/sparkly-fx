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

}