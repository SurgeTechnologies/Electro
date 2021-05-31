//                     ELECTRO ENGINE
// -------------- Electro Engine PBR Shader --------------
//                   --- HLSL v5.0 ---
// Copyright(c) 2021 - Electro Team - All rights reserved
/*
 *       CBuffer Guide
 *Binding -  Name       -  Shader---------------------|
 *--------|-------------|-----------------------------|
 *   0    | Camera      | [PBR.hlsl]                  |
 *   1    | Mesh        | [PBR.hlsl]                  |
 *   2    | Material    | [PBR.hlsl]                  |
 *   3    | Lights      | [PBR.hlsl]                  |
 *   4    | Roughness   | [PreFilterConvolution.hlsl] |
 *   5    | Skybox      | [Skybox.hlsl]               |
 *   6    | LightMat    | [PBR.hlsl]                  |
 *   7    | CascadeEnds | [PBR.hlsl]                  |
 *----------------------------------------------------|
 */
#type vertex
#pragma pack_matrix(row_major)
static const int NUM_CASCADES = 3;

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
    float3 v_ClipSpacePosZ : M_CSPZ;
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

    for (int i = 0; i < NUM_CASCADES; i++)
        output.v_LightSpaceVector[i] = mul(temp, u_LightSpaceMatrix[i]);

    output.v_Position = mul(temp, u_ViewProjection);
    output.v_ClipSpacePosZ = mul(float4(output.v_WorldPos, 1.0), u_ViewMatrix);
    output.v_TexCoord = float2(input.a_TexCoord.x, input.a_TexCoord.y);
    return output;
}

#type pixel
#pragma pack_matrix(row_major)
static const float PI = 3.14159265359;
static const float3 Fdielectric = 0.04; // Constant normal incidence Fresnel factor for all dielectrics
static const float Gamma = 2.2;
static const int NUM_CASCADES = 3;

struct vsOut
{
    float4 v_Position  : SV_POSITION;
    float3 v_Normal    : M_NORMAL;
    float3 v_Tangent   : M_TANGENT;
    float3 v_Bitangent : M_BITANGENT;
    float2 v_TexCoord  : M_TEXCOORD;
    float3 v_WorldPos  : M_POSITION;
    float4 v_LightSpaceVector[NUM_CASCADES] : M_LSV;
    float3 v_ClipSpacePosZ : M_CSPZ;
};

cbuffer Material : register(b2)
{
    float3 Albedo;
    float Metallic;

    float Roughness;
    float AO;
    int AlbedoTexToggle;
    int MetallicTexToggle;

    int AOTexToggle;
    int RoughnessTexToggle;
    int NormalTexToggle;
    float __Padding0;
}

struct PointLight
{
    float3 Position;
    float Intensity;

    float3 Color;
    float __Padding1;
};

struct DirectionalLight
{
    float3 Direction;
    float Intensity;

    float3 Color;
    float __Padding1;
};

cbuffer CascadeEnds : register(b7)
{
    float u_CascadeEnds[NUM_CASCADES + 1];
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
SamplerState ShadowSampler : register(s2);

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

float CalculateShadows(int cascadeIndex, float4 lightSpaceVector, float3 normal, float3 direction)
{
    float3 ProjCoords = lightSpaceVector.xyz / lightSpaceVector.w;

    float2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;

    float z = 0.5 * ProjCoords.z + 0.5;
    float Depth = ShadowMap[cascadeIndex].Sample(ShadowSampler, UVCoords).x;

    if (Depth < z + 0.00001)
        return 0.5;
    else
        return 1.0;
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
    for (uint i = 0; i < u_PointLightCount; ++i)
    {
        float3 dir = u_PointLights[i].Position - input.v_WorldPos;
        float3 L = normalize(dir);
        float3 distance = length(dir);

        // Calculate attenuation and use it to get the radiance
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = attenuation * u_PointLights[i].Color;
        directLighting += CalculateLight(N, L, V, max(radiance, 0.0.xxx), params.Albedo, params.Roughness, params.Metallic) * u_PointLights[i].Intensity;
    }

    float4 CascadeIndicator = float4(0.0, 0.0, 0.0, 0.0);
    for (uint i = 0; i < u_DirectionalLightCount; ++i)
    {
        float3 contribution = float3(0.0, 0.0, 0.0);
        float3 L = normalize(u_DirectionalLights[i].Direction);
        float3 radiance = u_DirectionalLights[i].Color;

        float shadow = 1.0f;

        for (int j = 0; j < NUM_CASCADES; j++)
        {
            if (input.v_ClipSpacePosZ.z < u_CascadeEnds[j])
            {
                return float4(input.v_ClipSpacePosZ.x, input.v_ClipSpacePosZ.y, input.v_ClipSpacePosZ.z, 1.0f);
                shadow = CalculateShadows(0, input.v_LightSpaceVector[0], N, u_DirectionalLights[i].Direction);
                if (i == 0)
                    CascadeIndicator = float4(0.1, 0.0, 0.0, 0.0);
                else if (i == 1)
                    CascadeIndicator = float4(0.0, 0.1, 0.0, 0.0);
                else if (i == 2)
                    CascadeIndicator = float4(0.0, 0.0, 0.1, 0.0);
                break;
            }
        }

        contribution = CalculateLight(N, L, V, max(radiance, 0.0.xxx), params.Albedo, params.Roughness, params.Metallic) * u_DirectionalLights[i].Intensity;
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

    PixelColor = float4(color, albedoResult.a) + CascadeIndicator;
    return PixelColor;
}

