//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) { matrix u_ViewProjection; }

struct vsIn
{
    float3 aPosition : POSITION;
    float4 aColor : COLOR;
};

struct vsOut
{
    float4 aPosition : SV_POSITION;
    float4 aColor : COLOR;
};

vsOut main(vsIn input)
{
    vsOut output;
    output.aPosition = mul(float4(input.aPosition, 1.0f), u_ViewProjection);
    output.aColor = input.aColor;

    return output;
}

#type pixel
struct vsOut
{
    float4 aPosition : SV_POSITION;
    float4 aColor : COLOR;
};

float4 main(vsOut input) : SV_TARGET
{
    return input.aColor;
}



