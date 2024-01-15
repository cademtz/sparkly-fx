// Return depth encoded from [0...255]
#include "shared.hlsl"

sampler depth_tex : register(s0);
const float4x4 inv_projection : register(c0);
const float3 depth_params : register(c4); // depth_near, depth_far, spherical_correction [0,1]

float4 main(float2 uv : TEXCOORD0) : COLOR {
    float depth_near = depth_params[0];
    float depth_far = depth_params[1];
    float spherical_correction = depth_params[2];
    float depth = tex2D(depth_tex, uv).x;
    float3 pos = reconstructPositionWithZ(uv, depth, inv_projection);

    depth = lerp(pos.z, length(pos), spherical_correction);
    
    depth -= depth_near;
    depth /= depth_far;
    depth = clamp(depth, 0, 1);

    return float4(depth, depth, depth, 1.0);
}