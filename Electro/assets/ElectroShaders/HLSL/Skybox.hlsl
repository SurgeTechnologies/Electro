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
    float3 v_TexCoords : TEXCOORD0;
};

vsOut main(uint vID : SV_VERTEXID)
{
    vsOut output;
    output.v_TexCoords = CreateCube(vID) - float3(0.5, 0.5, 0.5);
    float4 pos = mul(float4(output.v_TexCoords, 1.0f), u_ViewProjection);
    output.v_Position = pos;
    return output;
}

#type pixel
struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_TexCoords : TEXCOORD0;
};

cbuffer SkyboxCbuffer : register(b5)
{
    float u_TextureLOD;
    float u_Intensity;
    float2 __Padding;
};

TextureCube SkyboxCubemap : register(t0);
SamplerState sampleType : register(s0);

float4 main(vsOut input) : SV_TARGET
{
    const float gamma = 2.2;
    float3 pixelColor = SkyboxCubemap.SampleLevel(sampleType, input.v_TexCoords, u_TextureLOD) * u_Intensity;

    // Tonemapping
    pixelColor = pixelColor / (pixelColor + float3(1.0, 1.0, 1.0));

    pixelColor = pow(pixelColor, float3((1.0 / gamma).xxx));

    return float4(pixelColor, 1.0f);
}
