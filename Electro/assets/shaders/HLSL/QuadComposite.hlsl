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
Texture2D blurredTexture : register(t0);

cbuffer BloomExposure : register(b12)
{
    float4 u_Exposure;
}

float4 main(VSOut input) : SV_TARGET
{
    float3 hdrColor = blurredTexture.Sample(texSampler, input.v_TexCoord).rgb * u_Exposure.x;
    return float4(hdrColor, 1.0);
}