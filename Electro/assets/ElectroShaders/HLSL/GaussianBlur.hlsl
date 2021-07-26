//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved

#type compute

#define GAUSSIAN_RADIUS 39

Texture2D inputTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);

cbuffer BlurParams : register(b10)
{
    float4 u_Coefficients[(GAUSSIAN_RADIUS + 1) / 4];
    int2 u_RadiusAndDirection;
    int2 __Padding;
}

[numthreads(8, 8, 1)]
void main(uint3 dispatchID : SV_DispatchThreadID)
{
    int2 pixel = int2(dispatchID.x, dispatchID.y);

    int radius = u_RadiusAndDirection.x;
    int2 dir = int2(1 - u_RadiusAndDirection.y, u_RadiusAndDirection.y);

    float4 accumulatedValue = float4(0.0.xxxx);

    for (int i = -radius; i <= radius; ++i)
    {
        uint cIndex = (uint)abs(i);
        accumulatedValue += u_Coefficients[cIndex >> 2][cIndex & 3] * inputTexture[mad(i, dir, pixel)];
    }

    outputTexture[pixel] = accumulatedValue;
}
