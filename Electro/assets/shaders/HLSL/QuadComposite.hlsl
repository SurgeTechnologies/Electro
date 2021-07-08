//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

#type vertex
#pragma pack_matrix(row_major)

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

VSOut main(uint vID : SV_VERTEXID)
{
    VSOut output;
    output.v_TexCoord = float2((vID << 1) & 2, vID & 2);
    output.v_Position = float4(output.v_TexCoord * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}

#type pixel

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

SamplerState texSampler : register(s1);

Texture2D sceneTexture : register(t0);
Texture2D blurredTexture : register(t1);

cbuffer BloomExposure : register(b12)
{
    float4 u_Exposure;
}

float3 ACESFitted(float3 color)
{
    const float3x3 ACESInputMat =
    {
        { 0.59719, 0.35458, 0.04823 },
        { 0.07600, 0.90834, 0.01566 },
        { 0.02840, 0.13383, 0.83777 }
    };

    const float3x3 ACESOutputMat =
    {
        { 1.60475, -0.53108, -0.07367 },
        { -0.10208, 1.10813, -0.00605 },
        { -0.00327, -0.07276, 1.07602 }
    };
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    float3 a = color * (color + 0.0245786f) - 0.000090537f;
    float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
    return a / b;

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);
    return color;
}

float3 GammaCorrect(float3 input, float gamma)
{
    return pow(input, float3((1.0 / gamma).xxx));
}

float4 main(VSOut input) : SV_TARGET
{
    float3 hdrColor = sceneTexture.Sample(texSampler, input.v_TexCoord).rgb;
    float3 bloomColor = blurredTexture.Sample(texSampler, input.v_TexCoord).rgb;

    hdrColor += bloomColor; // Additive blending
    hdrColor *= u_Exposure.x;

    float3 result = 0.0f;
    result = ACESFitted(hdrColor);
    result = GammaCorrect(result, 2.2);

    return float4(result, 1.0f);
}