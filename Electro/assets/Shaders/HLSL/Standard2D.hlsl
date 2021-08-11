#type vertex
#pragma pack_matrix(row_major)

cbuffer Data : register(b0)
{
    matrix uViewProjection; //Camera
}

struct vsIn
{
    float4 vPosition     : POSITION;
    float4 vColor        : COLOR;
    float2 vTexCoord     : TEXCOORD;
    float  vTexIndex     : TEXINDEX;
    float  vTilingFactor : TILINGFACTOR;
};

struct vsOut
{
    float4 vPosition     : SV_POSITION;
    float4 vColor        : COLOR;
    float2 vTexCoord     : TEXCOORD;
    float  vTexIndex     : TEXINDEX;
    float  vTilingFactor : TILINGFACTOR;
};

vsOut main(vsIn input)
{
    vsOut output;
    output.vPosition = mul(input.vPosition, uViewProjection);

    output.vColor = input.vColor;
    output.vTexCoord = input.vTexCoord;
    output.vTexIndex = input.vTexIndex;
    output.vTilingFactor = input.vTilingFactor;
    return output;
}

#type pixel
struct vsOut
{
    float4 vPosition     : SV_POSITION;
    float4 vColor        : COLOR;
    float2 vTexCoord     : TEXCOORD;
    float  vTexIndex     : TEXINDEX;
    float  vTilingFactor : TILINGFACTOR;
};

Texture2D textures[32];
SamplerState sampleType : register(s0);

float4 main(vsOut input) : SV_TARGET
{

    switch ((int)input.vTexIndex)
    {
        case  0: return textures[ 0].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  1: return textures[ 1].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  2: return textures[ 2].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  3: return textures[ 3].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  4: return textures[ 4].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  5: return textures[ 5].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  6: return textures[ 6].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  7: return textures[ 7].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  8: return textures[ 8].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case  9: return textures[ 9].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 10: return textures[10].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 11: return textures[11].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 12: return textures[12].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 13: return textures[13].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 14: return textures[14].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 15: return textures[15].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 16: return textures[16].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 17: return textures[17].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 18: return textures[18].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 19: return textures[19].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 20: return textures[20].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 21: return textures[21].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 22: return textures[22].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 23: return textures[23].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 24: return textures[24].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 25: return textures[25].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 26: return textures[26].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 27: return textures[27].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 28: return textures[28].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 29: return textures[29].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 30: return textures[30].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        case 31: return textures[31].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
        default: return textures[ 0].Sample(sampleType, input.vTexCoord * input.vTilingFactor) * input.vColor; break;
    }
}
