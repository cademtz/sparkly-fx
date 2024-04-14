// Return depth encoded from [0...255]
// Credits: Half-Life Advanced Effects contributors (advancedfx.org)
#include "shared.hlsl"

sampler depth_tex : register(s0);
const float4x4 inv_projection : register(c0);
const float4 depth_params : register(c4); // depth_near, depth_far, spherical_correction [0,1], near_precision [1, inf)

float4 main(float4 uv : COLOR0) : COLOR {
    float depth_near = depth_params[0];
    float depth_far = depth_params[1];
    float spherical_correction = depth_params[2];
    float near_precision = depth_params[3];
    float depth = tex2D(depth_tex, uv.xy).x;
    float3 pos = reconstructPositionWithZ(uv.xy, depth, inv_projection);

    depth = lerp(pos.z, length(pos), spherical_correction);

    depth -= depth_near;
    depth = max(0, depth);
    depth = log(1 + depth * near_precision) / log(1 + (depth_far - depth_near) * near_precision);
    return float4(depth, depth, depth, 1.0);
}
