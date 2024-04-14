// Return depth encoded as 0xRRGGBB
#include "shared.hlsl"

sampler depth_tex : register(s0);
const float4x4 inv_projection : register(c0);

float4 main(float4 uv : COLOR0) : COLOR {
    float depth = tex2D(depth_tex, uv.xy).x;
    float3 depth_rgb = depth * 16777215.0;
    depth_rgb /= float3(16777215.0, 65535.0, 255.0);
    depth_rgb = frac(depth_rgb);
    return float4(depth_rgb, 1.0);
}