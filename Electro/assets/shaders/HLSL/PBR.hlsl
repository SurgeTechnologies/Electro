//                     ELECTRO ENGINE
// -------------- Electro Engine PBR Shader --------------
//                   --- HLSL v5.0 ---
// Copyright(c) 2021 - Electro Team - All rights reserved
/*
 *       CBuffer Guide
 *Binding -  Name          -  Shader---------------------|
 *--------|----------------|-----------------------------|
 *   0    | Camera         | [PBR.hlsl]                  |
 *   1    | Mesh           | [PBR.hlsl]                  |
 *   2    | Material       | [PBR.hlsl]                  |
 *   3    | Lights         | [PBR.hlsl]                  |
 *   4    | Roughness      | [PreFilterConvolution.hlsl] |
 *   5    | Skybox         | [Skybox.hlsl]               |
 *   6    | LightMat       | [PBR.hlsl]                  |
 *   7    | ShadowSettings | [PBR.hlsl]                  |
 *-------------------------------------------------------|
 */
#type vertex
#pragma pack_matrix(row_major)
static const int NUM_CASCADES = 4;

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
cbuffer Mesh   : register(b1) { matrix u_Transform; }
cbuffer LightMat : register(b6)
{
    matrix u_LightSpaceMatrix[NUM_CASCADES];
    matrix u_ViewMatrix;
}

struct vsIn
{
    float3 a_Position  : M_POSITION;
    float3 a_Normal    : M_NORMAL;
    float3 a_Tangent   : M_TANGENT;
    float3 a_Bitangent : M_BITANGENT;
    float2 a_TexCoord  : M_TEXCOORD;
};

struct vsOut
{
    float4 v_Position  : SV_POSITION;
    float3 v_Normal    : M_NORMAL;
    float3 v_Tangent   : M_TANGENT;
    float3 v_Bitangent : M_BITANGENT;
    float2 v_TexCoord  : M_TEXCOORD;
    float3 v_WorldPos  : M_POSITION;
    float4 v_LightSpaceVector[NUM_CASCADES] : M_LSV;
    float3 v_ViewSpacePos : M_VIEW_SPACE_POS;
};

vsOut main(vsIn input)
{
    vsOut output;
    float4 temp = float4(input.a_Position, 1.0f);

    output.v_Normal    = mul(normalize(input.a_Normal),  (float3x3)u_Transform);
    output.v_Tangent   = mul(input.a_Tangent, (float3x3)u_Transform);
    output.v_Bitangent = mul(input.a_Bitangent, (float3x3)u_Transform);

    temp = mul(temp, u_Transform);
    output.v_WorldPos = temp.xyz;
    output.v_ViewSpacePos = mul(temp, u_ViewMatrix);

    for (int i = 0; i < NUM_CASCADES; i++)
        output.v_LightSpaceVector[i] = mul(temp, u_LightSpaceMatrix[i]);

    output.v_Position = mul(temp, u_ViewProjection);
    output.v_TexCoord = float2(input.a_TexCoord.x, input.a_TexCoord.y);
    return output;
}

#type pixel
#pragma pack_matrix(row_major)
static const float PI = 3.14159265359;
static const float3 Fdielectric = 0.04; // Constant normal incidence Fresnel factor for all dielectrics
static const float Gamma = 2.2;
static const int NUM_CASCADES = 4;
static const int PCF_SAMPLES = 7;

struct vsOut
{
    float4 v_Position  : SV_POSITION;
    float3 v_Normal    : M_NORMAL;
    float3 v_Tangent   : M_TANGENT;
    float3 v_Bitangent : M_BITANGENT;
    float2 v_TexCoord  : M_TEXCOORD;
    float3 v_WorldPos  : M_POSITION;
    float4 v_LightSpaceVector[NUM_CASCADES] : M_LSV;
    float3 v_ViewSpacePos : M_VIEW_SPACE_POS;
};

cbuffer Material : register(b2)
{
    float3 Albedo;
    float Metallic;

    float Roughness;
    float AO;
    int AlbedoTexToggle;
    float Emissive;

    int MetallicTexToggle;
    int AOTexToggle;
    int RoughnessTexToggle;
    int NormalTexToggle;
}

struct PointLight
{
    float3 Position;
    float Intensity;

    float3 Color;
    float Radius;
};

struct DirectionalLight
{
    float3 Direction;
    float Intensity;

    float3 Color;
    float __Padding1;
};

cbuffer ShadowSettings : register(b7)
{
    float4 u_CascadeEnds;
};

cbuffer Lights : register(b3)
{
    float3 u_CameraPosition;
    int __Padding1;

    int u_PointLightCount;
    int u_DirectionalLightCount;
    float2 __Padding2;

    PointLight u_PointLights[100];
    DirectionalLight u_DirectionalLights[4];
};

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor
float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float3 CalculateLight(float3 N, float3 L, float3 V, float3 radiance, float3 albedo, float roughness, float metalness)
{
    float3 H = normalize(V + L);

    float3 F0 = Fdielectric;
    F0 = lerp(F0, albedo, metalness);
    float cosTheta = max(dot(H, V), 0.0);
    float3 F = FresnelSchlick(F0, cosTheta);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float3 kS = F;
    float3 kD = 1.0f - kS;

    kD *= 1.0 - metalness;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

struct PBRParameters
{
    float3 Albedo;
    float Metallic;
    float Roughness;
    float AO;
};

//Texture Maps - Linked with material
Texture2D AlbedoMap     : register(t0);
Texture2D NormalMap     : register(t1);
Texture2D MetallicMap   : register(t2);
Texture2D RoughnessMap  : register(t3);
Texture2D AOMap         : register(t4);

//IBL
TextureCube IrradianceMap : register(t5);
TextureCube PreFilterMap  : register(t6);
Texture2D BRDF_LUT : register(t7);

Texture2D ShadowMap[NUM_CASCADES] : register(t8);

//Sampler
SamplerState DefaultSampler : register(s0);
SamplerState BRDF_Sampler : register(s1);
SamplerComparisonState CMPSampler : register(s2);

// Returns number of mipmap levels for specular IBL environment map.
uint QuerySpecularTextureLevels()
{
    uint width, height, levels;
    PreFilterMap.GetDimensions(0, width, height, levels);
    return levels;
}

float3 CalculateNormalFromMap(float3 normal, float3 tangent, float3 bitangent, float2 texCoords)
{
    float3 Normal = normalize(normal);
    float3 Tangent = normalize(tangent);
    float3 Bitangent = normalize(bitangent);
    float3 BumpMapNormal = NormalMap.Sample(DefaultSampler, texCoords).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - float3(1.0, 1.0, 1.0);
    float3 NewNormal;
    float3x3 TBN = float3x3(Tangent, Bitangent, Normal);
    NewNormal = mul(transpose(TBN), BumpMapNormal);
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

static const float2 PoissonDisk[64] =
{
    float2(-0.5119625f, -0.4827938f),
    float2(-0.2171264f, -0.4768726f),
    float2(-0.7552931f, -0.2426507f),
    float2(-0.7136765f, -0.4496614f),
    float2(-0.5938849f, -0.6895654f),
    float2(-0.3148003f, -0.7047654f),
    float2(-0.42215f, -0.2024607f),
    float2(-0.9466816f, -0.2014508f),
    float2(-0.8409063f, -0.03465778f),
    float2(-0.6517572f, -0.07476326f),
    float2(-0.1041822f, -0.02521214f),
    float2(-0.3042712f, -0.02195431f),
    float2(-0.5082307f, 0.1079806f),
    float2(-0.08429877f, -0.2316298f),
    float2(-0.9879128f, 0.1113683f),
    float2(-0.3859636f, 0.3363545f),
    float2(-0.1925334f, 0.1787288f),
    float2(0.003256182f, 0.138135f),
    float2(-0.8706837f, 0.3010679f),
    float2(-0.6982038f, 0.1904326f),
    float2(0.1975043f, 0.2221317f),
    float2(0.1507788f, 0.4204168f),
    float2(0.3514056f, 0.09865579f),
    float2(0.1558783f, -0.08460935f),
    float2(-0.0684978f, 0.4461993f),
    float2(0.3780522f, 0.3478679f),
    float2(0.3956799f, -0.1469177f),
    float2(0.5838975f, 0.1054943f),
    float2(0.6155105f, 0.3245716f),
    float2(0.3928624f, -0.4417621f),
    float2(0.1749884f, -0.4202175f),
    float2(0.6813727f, -0.2424808f),
    float2(-0.6707711f, 0.4912741f),
    float2(0.0005130528f, -0.8058334f),
    float2(0.02703013f, -0.6010728f),
    float2(-0.1658188f, -0.9695674f),
    float2(0.4060591f, -0.7100726f),
    float2(0.7713396f, -0.4713659f),
    float2(0.573212f, -0.51544f),
    float2(-0.3448896f, -0.9046497f),
    float2(0.1268544f, -0.9874692f),
    float2(0.7418533f, -0.6667366f),
    float2(0.3492522f, 0.5924662f),
    float2(0.5679897f, 0.5343465f),
    float2(0.5663417f, 0.7708698f),
    float2(0.7375497f, 0.6691415f),
    float2(0.2271994f, -0.6163502f),
    float2(0.2312844f, 0.8725659f),
    float2(0.4216993f, 0.9002838f),
    float2(0.4262091f, -0.9013284f),
    float2(0.2001408f, -0.808381f),
    float2(0.149394f, 0.6650763f),
    float2(-0.09640376f, 0.9843736f),
    float2(0.7682328f, -0.07273844f),
    float2(0.04146584f, 0.8313184f),
    float2(0.9705266f, -0.1143304f),
    float2(0.9670017f, 0.1293385f),
    float2(0.9015037f, -0.3306949f),
    float2(-0.5085648f, 0.7534177f),
    float2(0.9055501f, 0.3758393f),
    float2(0.7599946f, 0.1809109f),
    float2(-0.2483695f, 0.7942952f),
    float2(-0.4241052f, 0.5581087f),
    float2(-0.1020106f, 0.6724468f),
};

float2 SamplePoission(int index)
{
    return PoissonDisk[index % 64];
}

float CalculateShadows(int cascadeIndex, float4 lightSpaceVector, float3 normal, float3 direction)
{
    // We need to do Perspective divide ourselves, as it is not done by the RenderAPI
    float3 projCoords = lightSpaceVector.xyz / lightSpaceVector.w;

    // Transform to [0,1] range
    projCoords.x = projCoords.x / 2 + 0.5;
    projCoords.y = projCoords.y / -2 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Fixes the shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, direction)), 0.0005);

    float shadow = 0.0;
    uint width, height, levels;
    ShadowMap[cascadeIndex].GetDimensions(0, width, height, levels);
    float2 texelSize = 1.0 / float2(width, height);

    for (uint s = 0; s < PCF_SAMPLES; s++)
        shadow += ShadowMap[cascadeIndex].SampleCmpLevelZero(CMPSampler, projCoords.xy + PoissonDisk[s] * texelSize * 3.0f, currentDepth - bias).r;

    shadow /= (float)PCF_SAMPLES;

    // Force the shadow value to 1.0 whenever the projected vector's z coordinate is larger than 1.0
    // -> Prevents Overdraw
    if (projCoords.z > 1.0)
        shadow = 1.0;

    return shadow;
}

float4 main(vsOut input) : SV_TARGET
{
    PBRParameters params;
    float4 PixelColor;

    float4 albedoResult;
    if (AlbedoTexToggle == 1)
    {
        albedoResult = AlbedoMap.Sample(DefaultSampler, input.v_TexCoord);
        params.Albedo = pow(albedoResult.rgb, Gamma);
    }
    else
    {
        albedoResult = float4(Albedo.x, Albedo.y, Albedo.z, 1.0f);
        params.Albedo = Albedo;
    }

    params.Albedo.x *= Emissive;
    params.Albedo.y *= Emissive;
    params.Albedo.z *= Emissive;

    params.Metallic  = MetallicTexToggle  == 1 ? MetallicMap.Sample(DefaultSampler, input.v_TexCoord).r  : Metallic;
    params.Roughness = RoughnessTexToggle == 1 ? RoughnessMap.Sample(DefaultSampler, input.v_TexCoord).r : Roughness;
    params.AO        = AOTexToggle        == 1 ? AOMap.Sample(DefaultSampler, input.v_TexCoord).r        : AO;

    float3 N = normalize(input.v_Normal);
    if (NormalTexToggle == 1)
        N = CalculateNormalFromMap(input.v_Normal, input.v_Tangent, input.v_Bitangent, input.v_TexCoord);

    // Outgoing light direction (floattor from world-space fragment position to the "eye")
    float3 V = normalize(u_CameraPosition - input.v_WorldPos);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, params.Albedo, params.Metallic);

    // Direct lighting calculation for analytical lights
    float3 directLighting = 0.0;

    // Point lights
    for (uint p = 0; p < u_PointLightCount; ++p)
    {
        float3 dir = u_PointLights[p].Position - input.v_WorldPos;
        float3 L = normalize(dir);
        float3 distance = length(dir);

        // Calculate attenuation and use it to get the radiance
        float attenuation = clamp(1.0 - (distance * distance) / (u_PointLights[p].Radius * u_PointLights[p].Radius), 0.0, 1.0);

        float3 radiance = u_PointLights[p].Color * u_PointLights[p].Intensity * attenuation;
        directLighting += CalculateLight(N, L, V, max(radiance, 0.0.xxx), params.Albedo, params.Roughness, params.Metallic);
    }

    // Directional Light
    for (uint i = 0; i < u_DirectionalLightCount; ++i)
    {
        float3 contribution = float3(0.0.xxx);
        float3 L = normalize(u_DirectionalLights[i].Direction);
        float3 radiance = u_DirectionalLights[i].Color;
        contribution = CalculateLight(N, L, V, max(radiance, 0.0.xxx), params.Albedo, params.Roughness, params.Metallic) * u_DirectionalLights[i].Intensity;

        float shadow = 1.0f;
        for (int j = 0; j < NUM_CASCADES; j++)
        {
            if (input.v_ViewSpacePos.z > -u_CascadeEnds[j])
            {
                int cascadeIndex = j;
                shadow = CalculateShadows(cascadeIndex, input.v_LightSpaceVector[cascadeIndex], N, u_DirectionalLights[i].Direction);
                break;
            }
        }
        directLighting += contribution * shadow;
    }

    // Ambient lighting (IBL)
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, params.Roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - params.Metallic;

    float3 irradiance = IrradianceMap.Sample(DefaultSampler, N).rgb;
    float3 diffuse = irradiance * params.Albedo;

    // Sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part
    float3 prefilteredColor = PreFilterMap.SampleLevel(DefaultSampler, reflect(-V, N), params.Roughness * QuerySpecularTextureLevels()).rgb;
    float2 brdf = BRDF_LUT.Sample(BRDF_Sampler, float2(max(dot(N, V), 0.0), params.Roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    float3 ambientLightning = (kD * diffuse + specular) * params.AO;

    float3 color = ambientLightning + directLighting;

    // HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
    // Gamma correction
    color = pow(color, float3(1.0 / Gamma, 1.0 / Gamma, 1.0 / Gamma));

    PixelColor = float4(color, albedoResult.a);
    return PixelColor;
}


