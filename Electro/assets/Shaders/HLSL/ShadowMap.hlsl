//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
cbuffer Mesh   : register(b1) { matrix u_Transform; }

struct vsIn  { float3 a_Position : M_POSITION; };
struct vsOut { float4 v_Position : SV_POSITION; };

vsOut main(vsIn input)
{
    vsOut output;
    output.v_Position = mul(float4(input.a_Position, 1.0f), mul(u_Transform, u_ViewProjection));
    return output;
}

#type pixel
struct vsOut
{
    float4 v_Position : SV_POSITION;
};

void main(vsOut input) {}
