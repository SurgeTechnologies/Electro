//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

#type vertex
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) { matrix u_ViewProjection; }

struct vsIn
{
    float3 a_Position : SKYBOX_POS;
};

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_TexCoords : TEX_COORDS;
};

vsOut main(vsIn input)
{
    vsOut output;
    output.v_TexCoords = input.a_Position;
    float4 pos = mul(float4(input.a_Position, 1.0f), u_ViewProjection);
    output.v_Position = pos.xyww;
    return output;
}

#type pixel
struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_TexCoords : TEX_COORDS;
};

cbuffer SkyboxCbuffer : register(b5)
{
    float u_TextureLOD;
    float u_Intensity;
    float2 __Padding;
};

TextureCube SkyboxCubemap : register(t32);
SamplerState sampleType : register(s0);

float4 main(vsOut input) : SV_TARGET
{
    float3 PixelColor = SkyboxCubemap.SampleLevel(sampleType, input.v_TexCoords, u_TextureLOD) * u_Intensity;
    PixelColor = PixelColor / (PixelColor + float3(1.0, 1.0, 1.0));
    PixelColor = pow(PixelColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(PixelColor, 1.0f);
}
