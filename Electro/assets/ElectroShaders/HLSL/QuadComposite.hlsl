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

Texture2D sceneTexture : register(t0);   // Geometry Texture [Main Scene]
Texture2D blurredTexture : register(t1); // Bloom Result [PostProcessing - Blurred Texture]

cbuffer CompositeParams : register(b12)
{
    float u_Exposure;
    int u_TonemappingAlgorithm; // See #defines below
    float2 u_InverseScreenSize;

    int u_ApplyFXAA;
    int3 __Padding;
}

/* TonemappingAlgorithms */
#define REINHARD 0
#define UNCHARTED2 1
#define REINHARD_JODIE 2
#define ACES_APPROX 3
#define ACES_FITTED 4

float3 GammaCorrect(float3 input, float gamma)
{
    return pow(input, float3((1.0 / gamma).xxx));
}

float3 Uncharted2TonemapPartial(float3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2Filmic(float3 v)
{
    float exposure_bias = 2.0f;
    float3 curr = Uncharted2TonemapPartial(v * exposure_bias);

    float3 W = float3(11.2f.xxx);
    float3 white_scale = float3(1.0f.xxx) / Uncharted2TonemapPartial(W);
    return curr * white_scale;
}

// Reinhard
float3 Reinhard(float3 v)
{
    return v / (1.0f + v);
}

// Reinhard Jodie
float3 ReinhardJodie(float3 v)
{
    float l = dot(v, float3(0.2126f, 0.7152f, 0.0722f));
    float3 tv = v / (1.0f + v);
    return lerp(v / (1.0f + l), tv, tv);
}

float3 AcesApprox(float3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}

float3 AcesFitted(float3 v)
{
    float3x3 aces_input_matrix =
    {
        float3(0.59719f, 0.35458f, 0.04823f),
        float3(0.07600f, 0.90834f, 0.01566f),
        float3(0.02840f, 0.13383f, 0.83777f)
    };
    
    float3x3 aces_output_matrix =
    {
        float3(1.60475f, -0.53108f, -0.07367f),
        float3(-0.10208f, 1.10813f, -0.00605f),
        float3(-0.00327f, -0.07276f, 1.07602f)
    };

    v = mul(aces_input_matrix, v);
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    v = a / b;
    return mul(aces_output_matrix, v);
}

// FXAA - Fast approximate anti aliasing

float RGBToLuma(float3 rgb)
{
    return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
}

float4 FXAATexOffset(Texture2D tex, float2 uv, int2 off)
{
    return tex.SampleLevel(texSampler, uv, 0.0, off.xy);
}

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.79

float3 ApplyFXAA(float3 hdrColor, float2 texCoords)
{
    // Luma at the current fragment
    float lumaCenter = RGBToLuma(hdrColor);

    // Luma at the four direct neighbours of the current fragment
    float lumaDown  = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2( 0.0, -1.0)).rgb);
    float lumaUp    = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2( 0.0,  1.0)).rgb);
    float lumaLeft  = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2(-1.0,  0.0)).rgb);
    float lumaRight = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2( 1.0,  0.0)).rgb);

    // Find the maximum and minimum luma around the current fragment
    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

    // Compute the delta
    float lumaRange = lumaMax - lumaMin;

    // If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA
    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
    {
        return hdrColor;
    }

    // Query the 4 remaining corners lumas
    float lumaDownLeft  = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2(-1, -1)).rgb);
    float lumaUpRight   = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2( 1,  1)).rgb);
    float lumaUpLeft    = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2(-1,  1)).rgb);
    float lumaDownRight = RGBToLuma(FXAATexOffset(sceneTexture, texCoords, float2( 1, -1)).rgb);

    // Combine the four edges lumas (using intermediary variables for future computations with the same values)
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;

    // Same for corners
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;

    // Compute an estimation of the gradient along the horizontal and vertical axis.
    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);

    // Is the local edge horizontal or vertical?
    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    // Select the two neighboring texels lumas in the opposite direction to the local edge
    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;

    // Compute gradients in this direction
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    // Which direction is the steepest?
    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    // Gradient in the corresponding direction, normalized
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
    
    // Choose the step size (one pixel) according to the edge direction.
    float stepLength = isHorizontal ? u_InverseScreenSize.y : u_InverseScreenSize.x;

    // Average luma in the correct direction
    float lumaLocalAverage = 0.0;

    if (is1Steepest)
    {
        // Switch the direction
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);

    // Shift UV in the correct direction by half a pixel
    float2 currentUv = texCoords;
    if (isHorizontal)
        currentUv.y += stepLength * 0.5;
    else
        currentUv.x += stepLength * 0.5;
    
    // Compute offset (for each iteration step) in the right direction.
    float2 offset = isHorizontal ? float2(u_InverseScreenSize.x, 0.0) : float2(0.0, u_InverseScreenSize.y);
    // Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;

    // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
    float lumaEnd1 = RGBToLuma(sceneTexture.Sample(texSampler, uv1).rgb);
    float lumaEnd2 = RGBToLuma(sceneTexture.Sample(texSampler, uv2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // If the side is not reached, we continue to explore in this direction.
    if (!reached1)
        uv1 -= offset;
    if (!reached2)
        uv2 += offset;

    // If both sides have not been reached, continue to explore
    if (!reachedBoth)
    {
        for (int i = 2; i < ITERATIONS; i++)
        {
            // If needed, read luma in 1st direction, compute delta
            if (!reached1)
            {
                lumaEnd1 = RGBToLuma(sceneTexture.Sample(texSampler, uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            // If needed, read luma in opposite direction, compute delta.
            if (!reached2)
            {
                lumaEnd2 = RGBToLuma(sceneTexture.Sample(texSampler, uv2).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }
            // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            // If the side is not reached, we continue to explore in this direction, with a variable quality
            if (!reached1)
                uv1 -= offset * QUALITY(i);
            if (!reached2)
                uv2 += offset * QUALITY(i);

            // If both sides have been reached, stop the exploration
            if (reachedBoth)
                break;
        }
    }

    // Compute the distances to each extremity of the edge
    float distance1 = isHorizontal ? (texCoords.x - uv1.x) : (texCoords.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - texCoords.x) : (uv2.y - texCoords.y);

    // In which direction is the extremity of the edge closer?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    // Length of the edge
    float edgeThickness = (distance1 + distance2);

    // UV offset: read in the direction of the closest side of the edge
    float pixelOffset = -distanceFinal / edgeThickness + 0.5;

    // Is the luma at center smaller than the local average?
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

    // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation)
    // (in the direction of the closer side of the edge)
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    // If the luma variation is incorrect, do not offset.
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // -------- Sub-pixel shifting --------
    // Full weighted average of the luma over the 3x3 neighborhood
    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    // Compute a sub-pixel offset based on this delta
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    finalOffset = max(finalOffset, subPixelOffsetFinal);

    float2 finalUv = texCoords;
    if (isHorizontal)
        finalUv.y += finalOffset * stepLength;
    else
        finalUv.x += finalOffset * stepLength;

    float3 finalColor = sceneTexture.Sample(texSampler, finalUv).rgb;
    return finalColor;
}


float4 main(VSOut input) : SV_TARGET
{
    float3 hdrColor = sceneTexture.Sample(texSampler, input.v_TexCoord).rgb;
    float3 bloomColor = blurredTexture.Sample(texSampler, input.v_TexCoord).rgb;

    if (u_ApplyFXAA == 1)
        hdrColor = ApplyFXAA(hdrColor, input.v_TexCoord);

    hdrColor += bloomColor; // Additive blending
    hdrColor *= u_Exposure.x;

    float3 result = 0.0f;

    if (u_TonemappingAlgorithm == ACES_FITTED)
        result = AcesFitted(hdrColor);
    else if (u_TonemappingAlgorithm == ACES_APPROX)
        result = AcesApprox(hdrColor);
    else if (u_TonemappingAlgorithm == REINHARD)
        result = Reinhard(hdrColor);
    else if (u_TonemappingAlgorithm == REINHARD_JODIE)
        result = ReinhardJodie(hdrColor);
    else if (u_TonemappingAlgorithm == UNCHARTED2)
        result = Uncharted2Filmic(hdrColor);

    result = GammaCorrect(result, 2.2);

    return float4(result, 1.0f);
}