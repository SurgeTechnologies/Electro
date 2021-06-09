//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)

struct VSIn
{
    float3 a_Position : POSITION;
    float2 a_TexCoord : TEXCOORD;
};

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.v_Position = float4(input.a_Position, 1.0);
    output.v_TexCoord = input.a_TexCoord;
    return output;
}

#type pixel
Texture2D texture0 : register(t12);
SamplerState sampler0 : register(s0);

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

float SampleTexture(float2 uv, float2 pixeloffset)
{
    float samples[9];

    uint width, height;
    texture0.GetDimensions(width, height);
    float2 pixelSize = 1.0f / float2(width, height);

    samples[0] = texture0.Sample(sampler0, uv + (float2(-1, -1) + pixeloffset) * pixelSize).r;
    samples[1] = texture0.Sample(sampler0, uv + (float2(+0, -1) + pixeloffset) * pixelSize).r;
    samples[2] = texture0.Sample(sampler0, uv + (float2(+1, -1) + pixeloffset) * pixelSize).r;
    samples[3] = texture0.Sample(sampler0, uv + (float2(-1, +0) + pixeloffset) * pixelSize).r;
    samples[4] = texture0.Sample(sampler0, uv + (float2(+0, +0) + pixeloffset) * pixelSize).r;
    samples[5] = texture0.Sample(sampler0, uv + (float2(+1, +0) + pixeloffset) * pixelSize).r;
    samples[6] = texture0.Sample(sampler0, uv + (float2(-1, +1) + pixeloffset) * pixelSize).r;
    samples[7] = texture0.Sample(sampler0, uv + (float2(+0, +1) + pixeloffset) * pixelSize).r;
    samples[8] = texture0.Sample(sampler0, uv + (float2(+1, +1) + pixeloffset) * pixelSize).r;

    float maxVal = 0.0f;
    for (int i = 0; i < 9; i++)
        maxVal = max(maxVal, samples[i]);

    return maxVal;
}

float4 main(VSOut input) : SV_Target
{
    float val = 0.0f;
    val += SampleTexture(input.v_TexCoord, 0.0f.xx);
    float3 color = float3(1.0, 0.5, 0.0);
    float multiplier = 1.0f - texture0.Sample(sampler0, input.v_TexCoord).r;
    val *= multiplier;
    return float4(color, val);
}
