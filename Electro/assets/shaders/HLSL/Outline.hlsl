//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#type vertex
#pragma pack_matrix(row_major)

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

VSOut main(uint vID : SV_VertexID)
{
    VSOut output;

    //https://wallisc.github.io/rendering/2021/04/18/Fullscreen-Pass.html
    output.v_TexCoord = float2((vID << 1) & 2, vID & 2);
    output.v_Position = float4(output.v_TexCoord * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}

#type pixel
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s3);

struct VSOut
{
    float4 v_Position : SV_POSITION;
    float2 v_TexCoord : TEXCOORD;
};

float SampleTexture(float2 uv, float2 pixeloffset)
{
    uint sampleCount = 9;
    float samples[9];

    uint width, height;
    texture0.GetDimensions(width, height);
    float2 pixelSize = 1.0f / float2(width, height);
    int pixelJump = 3;
    samples[0] = texture0.Sample(sampler0, uv + (float2(-pixelJump, -pixelJump) + pixeloffset) * pixelSize).r;
    samples[1] = texture0.Sample(sampler0, uv + (float2(+0, -pixelJump) + pixeloffset) * pixelSize).r;
    samples[2] = texture0.Sample(sampler0, uv + (float2(+pixelJump, -pixelJump) + pixeloffset) * pixelSize).r;
    samples[3] = texture0.Sample(sampler0, uv + (float2(-pixelJump, +0) + pixeloffset) * pixelSize).r;
    samples[4] = texture0.Sample(sampler0, uv + (float2(+0, +0) + pixeloffset) * pixelSize).r;
    samples[5] = texture0.Sample(sampler0, uv + (float2(+pixelJump, +0) + pixeloffset) * pixelSize).r;
    samples[6] = texture0.Sample(sampler0, uv + (float2(-pixelJump, +pixelJump) + pixeloffset) * pixelSize).r;
    samples[7] = texture0.Sample(sampler0, uv + (float2(+0, +pixelJump) + pixeloffset) * pixelSize).r;
    samples[8] = texture0.Sample(sampler0, uv + (float2(+pixelJump, +pixelJump) + pixeloffset) * pixelSize).r;

    float maxVal = 0.0f;
    for (uint i = 0; i < sampleCount; i++)
        maxVal = max(maxVal, samples[i]);

    return maxVal;
}

float4 main(VSOut input) : SV_TARGET
{
    float val = 0.0f;
    val += SampleTexture(input.v_TexCoord, 0.0f.xx);
    float3 color = float3(1.0, 0.4, 0.0);
    float multiplier = 1.0f - texture0.Sample(sampler0, input.v_TexCoord).r;
    val *= multiplier;
    return float4(color, val);
}
