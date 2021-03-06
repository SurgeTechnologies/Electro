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

TextureCube EnvironmentMap : register(t30);
SamplerState CubemapSampler : register(s1);

static const float PI = 3.14159265359;
float4 main(vsOut input) : SV_TARGET
{
    float3 N = normalize(input.v_LocalPos);
    float3 irradiance = 0.0;

    // Tangent space calculation from origin point
    float3 up    = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // Tangent space to world
            float3 samplefloat = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += EnvironmentMap.Sample(CubemapSampler, samplefloat).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    return float4(irradiance, 1.0);
}
