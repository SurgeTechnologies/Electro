//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
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
static const float PI = 3.14159265359;

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_LocalPos : POSITION;
};

TextureCube environmentMap : register(t31);
SamplerState DefaultSampler : register(s0);

cbuffer Roughness : register(b4)
{
    float4 u_Roughness;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space floattor to world-space sample floattor
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 samplefloat = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(samplefloat);
}

float4 main(vsOut input) : SV_TARGET
{
    float3 N = normalize(input.v_LocalPos);
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    float3 prefilteredColor = 0.0;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, u_Roughness.x);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            prefilteredColor += environmentMap.Sample(DefaultSampler, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    return float4(prefilteredColor, 1.0);
}
