//                     ELECTRO ENGINE
// -------------- Electro Engine PBR Shader --------------
//                   --- HLSL v5.0 ---
// Copyright(c) 2021 - Electro Team - All rights reserved

#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
cbuffer Mesh   : register(b1) { matrix u_Transform; }

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
    float4 v_Position : SV_POSITION;
    float3 v_Normal   : M_NORMAL;
    float2 v_TexCoord : M_TEXCOORD;
    float3 v_WorldPos : M_POSITION;
    float3x3 v_TangentBasis : TBASIS;
};

vsOut main(vsIn input)
{
    vsOut output;

    // Pass tangent space basis vectors (for normal mapping)
    float3x3 TBN = float3x3(input.a_Tangent, input.a_Bitangent, input.a_Normal);
    output.v_TangentBasis = mul((float3x3) u_Transform, TBN);
    output.v_Normal = mul(float4(input.a_Normal, 0.0f), u_Transform);

    float4 temp = float4(input.a_Position, 1.0f);
    temp = mul(temp, u_Transform);
    output.v_WorldPos = temp.xyz;
    output.v_Position = mul(temp, u_ViewProjection);
    
    output.v_TexCoord = input.a_TexCoord;
    return output;
}

#type pixel
static const float PI = 3.14159265359;
static const float Epsilon = 0.00001;
static const float3 Fdielectric = 0.04; // Constant normal incidence Fresnel factor for all dielectrics.
static const float Gamma = 2.2;

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_Normal   : M_NORMAL;
    float2 v_TexCoord : M_TEXCOORD;
    float3 v_WorldPos : M_POSITION;
    float3x3 v_TangentBasis : TBASIS;
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

cbuffer Lights : register(b3)
{
    float3 u_CameraPosition;
    int __Padding1;

    int u_PointLightCount;
    float3 __Padding2;

    PointLight u_PointLights[4];
};

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, Epsilon); // prevent divide by zero for roughness = 0.0 and NdotH = 1.0
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

struct PBRParameters
{
    float3 Albedo;
    float Metallic;
    float Roughness;
    float AO;
};
//Texture Maps
Texture2D AlbedoMap     : register(t0);
Texture2D NormalMap     : register(t1);
Texture2D MetallicMap   : register(t2);
Texture2D RoughnessMap  : register(t3);
Texture2D AOMap         : register(t4);

//Default Sampler
SamplerState DefaultSampler : register(s0);

float4 main(vsOut input) : SV_TARGET
{
    PBRParameters params;
    float4 PixelColor;
    params.Albedo    = AlbedoTexToggle    == 1 ? pow(AlbedoMap.Sample(DefaultSampler, input.v_TexCoord).rgb, Gamma)  : Albedo;
    params.Metallic  = MetallicTexToggle  == 1 ? MetallicMap.Sample(DefaultSampler, input.v_TexCoord).r              : Metallic;
    params.Roughness = RoughnessTexToggle == 1 ? RoughnessMap.Sample(DefaultSampler, input.v_TexCoord).r             : Roughness;
    params.AO        = AOTexToggle        == 1 ? AOMap.Sample(DefaultSampler, input.v_TexCoord).r                    : AO;

    float3 N = normalize(input.v_Normal);
    if (NormalTexToggle == 1)
    {
        N = normalize(2.0 * NormalMap.Sample(DefaultSampler, input.v_TexCoord).rgb - 1.0);
        N = normalize(mul(input.v_TangentBasis, N));
    }

    // Outgoing light direction (vector from world-space fragment position to the "eye")
    float3 V = normalize(u_CameraPosition - input.v_WorldPos); //input.v_WorldPos is the pixel position

    // Fresnel reflectance at normal incidence (for metals use albedo color)
    float3 F0 = lerp(Fdielectric, params.Albedo, params.Metallic);

    //Reflectance equation
    float3 Lo = 0.0;
    for(int i = 0; i < u_PointLightCount; ++i)
    {
        // Calculate per-light radiance
        float3 L = normalize(u_PointLights[i].Position - input.v_WorldPos);
        float3 H = normalize(V + L);
        float distance = length(u_PointLights[i].Position - input.v_WorldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = u_PointLights[i].Color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, params.Roughness); // Calculate normal distribution for specular BRDF
        float G = GeometrySmith(N, V, L, params.Roughness); // Calculate Geometric attenuation for specular BRDF
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0); // Calculate Fresnel term for direct lighting

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - params.Metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        float3 specular = numerator / max(denominator, 0.001);

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * params.Albedo / PI + specular) * radiance * NdotL * u_PointLights[i].Intensity;
    }
 
    float3 ambient = float3(0.03, 0.03, 0.03) * params.Albedo * params.AO;
    float3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));

    // Gamma correction
    color = pow(color, float3(1.0 / Gamma, 1.0 / Gamma, 1.0 / Gamma));

    PixelColor = float4(color, 1.0);
    return PixelColor;
}
