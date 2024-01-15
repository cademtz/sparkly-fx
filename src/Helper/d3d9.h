#pragma once
#include "dxerr.h"
#include <array>
#include <cstdint>
#include <d3d9.h>

namespace Helper
{

struct D3DFORMAT_info
{
    /// @brief Number of bytes between each pixel
    uint32_t stride;
    /// @brief The bit depth of each channel
    uint8_t bitdepth[4];
    /// @brief Number of channels
    uint8_t num_channels;
    /// @brief True if all bitdepths are equal
    bool is_uniform_bitdepth;
    bool is_float;
    bool is_depth;
};

static constexpr D3DFORMAT FOURCC_INTZ = (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z');
/// @brief This is a bogus format ID to represent 24-bit RGB.
/// @details Being for little-endian systems, this is technically BGR.
static constexpr D3DFORMAT D3DFMT_B8G8R8 = (D3DFORMAT)0x7FFFFFFF;

/**
 * @brief Get detailed information about a D3D surface/texture format
 * @param fmt One of the `D3DFMT_` or `FOURCC_` values
 * @param out_info Location to store the info
 * @return `true` if the info was returned
 */
bool GetD3DFormatInfo(D3DFORMAT fmt, D3DFORMAT_info* out_info);

/// @brief Get the equivalent FFmpeg `pix_fmt` for a given `D3DFORMAT`.
/// @details The chosen pix_fmt is in reverse order to account for little-endian.
/// @param fmt One of the `D3DFMT_` or `FOURCC_` values
/// @param skip_alpha If true, then it returns only equivalent formats that will skip the alpha channel
/// @return `nullptr` if no equivalent `pix_fmt` exists
const char* GetD3DFormatAsFFmpegPixFmt(D3DFORMAT fmt, bool skip_alpha);

/**
 * @brief Invert a row-major (DirectX-style) matrix
 * @param m A 4x4 matrix
 * @param out Destination of the inverse 4x4 matrix. It can be the same as `m`.
 */
void Invert4x4Matrix(const float *m, float *out);

/**
 * @brief Draw a textured rectangle in screen-space.
 * 
 * Everything is unbound or modified except the pixel shader, shader constants, and textures.
 * You will want to create a stateblock to restore everything after this call.
 * 
 * @param screen_rect X,Y,W,H screen rectangle. Units are in pixels, with (0,0) being the top-left.
 * @param texture_rect X,Y,W,H texture rectangle. Units are within [0,1].
 */
void DrawTexturedRect(IDirect3DDevice9* device, std::array<float, 4> screen_rect, std::array<float, 4> texture_rect = {0,0,1,1});
}