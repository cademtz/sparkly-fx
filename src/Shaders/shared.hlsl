// Shared functions that are commonly used and I don't want to rewrite them for every file.

// Reconstruct the position in game units relative to the camera. The Z-axis will be constant.
float3 reconstructPosition(in float2 uv, in float z, in float4x4 InvVP) {
  float4 position_s = float4(uv * 2 - 1, z, 1.0f);
  float4 position_v = mul(InvVP, position_s);
  return position_v.xyz / position_v.w;
}

// Reconstruct the position in game units relative to the camera including the Z-axis
float3 reconstructPositionWithZ(in float2 uv, in float z, in float4x4 InvVP) {
  float4 position_s = float4(uv * 2 - 1, z, 1.0f);
  float4 position_v = mul(InvVP, position_s);
  position_v.z = 1.0;
  return position_v.xyz / position_v.w;
}