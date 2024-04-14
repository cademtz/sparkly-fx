// Return OpenGL-style geometry normals as color
#include "shared.hlsl"

sampler depth_tex : register(s0);
const float4x4 inv_projection : register(c0);

float4 main(float2 uv : COLOR0) : COLOR {
    float depth = tex2D(depth_tex, uv.xy).x;

    float3 pos = reconstructPosition(uv.xy, depth, inv_projection);
    float3 normal = normalize(cross(ddx(pos), ddy(pos)));
    normal.y = -normal.y; // The Y-axis is flipped to get traditional OpenGL normals
    normal = (normal + 1) / 2;
    return float4(normal, 1.0);
}