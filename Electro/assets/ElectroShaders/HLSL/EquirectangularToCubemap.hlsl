//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }

float3 CreateCube(uint vertexID)
{
    uint b = 1 << vertexID;
    float x = (0x287a & b) != 0;
    float y = (0x02af & b) != 0;
    float z = (0x31e3 & b) != 0;
    return float3(x, y, z);
}

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_LocalPos : POSITION;
};

vsOut main(uint vID : SV_VERTEXID)
{
    vsOut output;
    float3 pos = CreateCube(vID) - float3(0.5, 0.5, 0.5);
    output.v_Position = mul(float4(pos, 1.0), u_ViewProjection);
    output.v_LocalPos = pos;
    return output;
}

#type pixel
struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_LocalPos : POSITION;
};

Texture2D EquirectangularMap : register(t0);
SamplerState DefaultSampler : register(s0);

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 main(vsOut input) : SV_TARGET
{
    float4 PixelColor;
    float2 uv = SampleSphericalMap(normalize(input.v_LocalPos));
    float3 color = EquirectangularMap.Sample(DefaultSampler, uv).rgb;
    PixelColor = float4(color, 1.0);
    return PixelColor;
} 
