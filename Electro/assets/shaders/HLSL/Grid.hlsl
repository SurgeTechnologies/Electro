//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
// Grid Shader - HLSL 5.0
#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }

matrix inverse(matrix m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

struct vsIn
{
    float3 a_Postion : POSITION;
};

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float3 v_NearPoint : NEAR_POINT;
    float3 v_FarPoint : FAR_POINT;
    matrix v_ViewProjection : VIEW_PROJECTION;
};

float3 UnprojectPoint(float x, float y, float z)
{
    matrix invVP = inverse(u_ViewProjection);
    float4 unprojectedPoint = mul(float4(x, y, z, 1.0), invVP);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

VSOut main(vsIn input)
{
    VSOut output;
    float3 val = input.a_Postion;
    output.v_ViewProjection = u_ViewProjection;
    output.v_Position = float4(val, 1.0f);

    output.v_NearPoint = UnprojectPoint(val.x, val.y, 0.0).xyz; // unprojecting on the near plane
    output.v_FarPoint = UnprojectPoint(val.x, val.y, 1.0).xyz; // unprojecting on the far plane
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
    return output;
}