//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
// Grid Shader - HLSL 5.0
#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
cbuffer InverseCamera : register(b8) { matrix u_InverseViewProjection; }

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float3 v_NearPoint : NEAR_POINT;
    float3 v_FarPoint : FAR_POINT;
    matrix v_ViewProjection : VIEW_PROJECTION;
};

float3 UnprojectPoint(float x, float y, float z)
{
    matrix invVP = u_InverseViewProjection;
    float4 unprojectedPoint = mul(float4(x, y, z, 1.0), invVP);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

VSOut main(uint vID : SV_VERTEXID)
{
    VSOut output;
    float2 uv = float2((vID << 1) & 2, vID & 2);
    float3 val = float3(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0);
    output.v_Position = float4(val, 1.0f);
    output.v_ViewProjection = u_ViewProjection;

    output.v_NearPoint = UnprojectPoint(val.x, val.y, 0.0).xyz; // Unprojecting on the near plane
    output.v_FarPoint = UnprojectPoint(val.x, val.y, 1.0).xyz; // Unprojecting on the far plane
    return output;
}

#type pixel
#pragma pack_matrix(row_major)

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float3 v_NearPoint : NEAR_POINT;
    float3 v_FarPoint : FAR_POINT;
    matrix v_ViewProjection : VIEW_PROJECTION;
};

float4 Grid(float3 fragPos3D, float scale)
{
    float2 coord = fragPos3D.xz * scale;
    float2 derivative = fwidth(coord);
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    float4 color = float4(0.2, 0.2, 0.2, 1.0 - min(min(grid.x, grid.y), 1.0));
    // Z axis
    if (fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // X axis
    if (fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}

float ComputeLinearDepth(float3 pos, matrix viewProjection)
{
    float near = 0.1;
    float far = 100.f;
    float4 clipSpacePos = mul(float4(pos.xyz, 1.0), viewProjection);
    float3 projCoords = clipSpacePos.xyz / clipSpacePos.w;

    float clip_space_depth = projCoords.z;
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far; // normalize
}

float ComputeDepth(float3 pos, matrix viewProjection)
{
    float4 clipSpacePos = mul(float4(pos, 1.0), viewProjection);
    clipSpacePos.xyz /= clipSpacePos.w;
    return clipSpacePos.z;
}

struct psOut
{
    float4 PixelColor : SV_TARGET;
    float Depth : SV_DEPTH;
};

psOut main(VSOut input)
{
    psOut output;

    float t = -input.v_NearPoint.y / (input.v_FarPoint.y - input.v_NearPoint.y);
    float3 fragPos3D = input.v_NearPoint + t * (input.v_FarPoint - input.v_NearPoint);
    output.Depth = ComputeDepth(fragPos3D, input.v_ViewProjection);

    float linearDepth = ComputeLinearDepth(fragPos3D, input.v_ViewProjection);
    float fading = max(0, (0.5 - linearDepth));

    output.PixelColor = (Grid(fragPos3D, 10) + Grid(fragPos3D, 1)) * float(t > 0);
    output.PixelColor.a *= fading;

    if (output.PixelColor.a == 0)
        discard;

    return output;
}