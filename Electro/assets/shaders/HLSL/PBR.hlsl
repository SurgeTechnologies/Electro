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
    float4 v_Position  : SV_POSITION;
    float3 v_Normal    : M_NORMAL;
    float3 v_Tangent   : M_TANGENT;
    float3 v_Bitangent : M_BITANGENT;
    float2 v_TexCoord  : M_TEXCOORD;
    float3 v_WorldPos  : M_POSITION;
};

vsOut main(vsIn input)
{
    vsOut output;
    float4 temp = float4(input.a_Position, 1.0f);

    output.v_Normal    = mul(input.a_Normal,  (float3x3)u_Transform);
    output.v_Tangent   = mul(input.a_Tangent, (float3x3)u_Transform);
    output.v_Bitangent = mul(input.a_Bitangent, (float3x3)u_Transform);

    temp = mul(temp, u_Transform);
    output.v_WorldPos = temp.xyz;
    output.v_Position = mul(temp, u_ViewProjection);

    output.v_TexCoord = float2(input.a_TexCoord.x, input.a_TexCoord.y);
    return output;
}

#type pixel
static const float PI = 3.14159265359;
static const float Epsilon = 0.00001;
static const float3 Fdielectric = 0.04; // Constant normal incidence Fresnel factor for all dielectrics.
static const float Gamma = 2.2;

struct vsOut
{
    float4 v_Position  : SV_POSITION;
    float3 v_Normal    : M_NORMAL;
    float3 v_Tangent   : M_TANGENT;
    float3 v_Bitangent : M_BITANGENT;
    float2 v_TexCoord  : M_TEXCOORD;
    float3 v_WorldPos  : M_POSITION;
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

    PointLight u_PointLights[100];
};

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NDfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below
float GASchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float GASchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights
    return GASchlickG1(cosLi, k) * GASchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor
float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

//IBL
TextureCube IrradianceMap : register(t5);
TextureCube PreFilterMap : register(t6);
Texture2D BRDF_LUT : register(t7);

//Default Sampler
SamplerState DefaultSampler : register(s0);
SamplerState BRDF_Sampler : register(s1);

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
        N = CalculateNormalFromMap(input.v_Normal, input.v_Tangent, input.v_Bitangent, input.v_TexCoord);

    // Outgoing light direction (floattor from world-space fragment position to the "eye")
    float3 Lo = normalize(u_CameraPosition - input.v_WorldPos);

    // Angle between surface normal and outgoing light direction
    float cosLo = max(0.0, dot(N, Lo));

    // Specular reflection floattor
    float3 Lr = 2.0 * cosLo * N - Lo;

    // Fresnel reflectance at normal incidence (for metals use albedo color)
    float3 F0 = lerp(Fdielectric, params.Albedo, params.Metallic);

    // Direct lighting calculation for analytical lights
    float3 directLighting = 0.0;
    for (uint i = 0; i < u_PointLightCount; ++i)
    {
        float3 Li = normalize(u_PointLights[i].Position - input.v_WorldPos);
        float3 Lradiance = u_PointLights[i].Color;

        // Half-floattor between Li and Lo.
        float3 Lh = normalize(Li + Lo);

        // Calculate angles between surface normal and various light floattors
        float cosLi = max(0.0, dot(N, Li));
        float cosLh = max(0.0, dot(N, Lh));

        // Calculate Fresnel term for direct lighting
        float3 F = FresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
        // Calculate normal distribution for specular BRDF
        float D = NDfGGX(cosLh, params.Roughness);
        // Calculate geometric attenuation for specular BRDF
        float G = GASchlickGGX(cosLi, cosLo, params.Roughness);

        // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
        // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
        // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
        float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), params.Metallic);

        // Lambert diffuse BRDF
        float3 diffuseBRDF = kd * params.Albedo;

        // Cook-Torrance specular microfacet BRDF
        float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

        // Total contribution for this light
        directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi * u_PointLights[i].Intensity;
    }

    // Ambient lighting (IBL).
    float3 ambientLighting;
    {
        // Sample diffuse irradiance at normal direction.
        float3 irradiance = IrradianceMap.Sample(DefaultSampler, N).rgb;

        // Calculate Fresnel term for ambient lighting.
        // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
        // use cosLo instead of angle with light's half-vector (cosLh above)
        // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
        float3 F = FresnelSchlick(F0, cosLo);

        // Get diffuse contribution factor (as with direct lighting)
        float3 kd = lerp(1.0 - F, 0.0, params.Metallic);

        // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either
        float3 diffuseIBL = kd * params.Albedo * irradiance;

        // Sample pre-filtered specular reflection environment at correct mipmap level.
        uint specularTextureLevels = QuerySpecularTextureLevels();
        float3 specularIrradiance = PreFilterMap.SampleLevel(DefaultSampler, Lr, params.Roughness * specularTextureLevels).rgb;

        // Split-sum approximation factors for Cook-Torrance specular BRDF.
        float2 specularBRDF = BRDF_LUT.Sample(BRDF_Sampler, float2(cosLo, params.Roughness)).rg;

        // Total specular IBL contribution.
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

        // Total ambient lighting contribution.
        ambientLighting = diffuseIBL + specularIBL;
    }
    float3 color = directLighting + ambientLighting;

    // HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
    // Gamma correction
    color = pow(color, float3(1.0 / Gamma, 1.0 / Gamma, 1.0 / Gamma));

    PixelColor = float4(color, 1.0);
    return PixelColor;
}
