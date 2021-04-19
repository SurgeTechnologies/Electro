//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0)
{
    matrix u_ViewProjection;
}

struct vsIn
{
    float3 a_Position : POSITION;
};

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_LocalPos : POSITION;
};

vsOut main(vsIn input)
{
    vsOut output;
    output.v_Position = mul(float4(input.a_Position, 1.0), u_ViewProjection);
    output.v_LocalPos = input.a_Position;

    return output;
}

#type pixel

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_LocalPos : POSITION;
};

TextureCube EnvironmentMap : register(t31); //Irradiance map is registered for slot 31
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
