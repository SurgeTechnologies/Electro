//                    SPIKE ENGINE
//Copyright 2021 - SpikeTechnologies - All Rights Reserved

#type vertex
#pragma pack_matrix(row_major)

cbuffer Camera : register(b0) { matrix u_ViewProjection; }
cbuffer Mesh   : register(b1) { matrix u_Transform; }

struct vsIn
{
    float3 a_Position : M_POSITION;
    float3 a_Normal   : M_NORMAL;
    float2 a_TexCoord : M_TEXCOORD;
};

struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_Normal   : M_NORMAL;
    float2 v_TexCoord : M_TEXCOORD;
    float3 v_WorldPos : M_POSITION;
};

vsOut main(vsIn input)
{
    vsOut output;

    float4 temp = float4(input.a_Position, 1);
    temp = mul(temp, u_Transform);
    output.v_Position = mul(temp, u_ViewProjection);
    output.v_WorldPos = temp.xyz;

    output.v_Normal = input.a_Normal;
    output.v_TexCoord = input.a_TexCoord;
    return output;
}

#type pixel
struct vsOut
{
    float4 v_Position : SV_POSITION;
    float3 v_Normal   : M_NORMAL;
    float2 v_TexCoord : M_TEXCOORD;
    float3 v_WorldPos : M_POSITION;
};

//Lights
struct SkyLight
{
    float3 Color;
    float Intensity;
};
struct PointLight
{
    float3 Position;
    float __Padding0;

    float3 Color;
    float __Padding1;

    float Intensity;
    float Constant;
    float Linear;
    float Quadratic;
};

cbuffer Material : register(b2)
{
    float3 u_MatColor;
    int u_MatDiffuseTexToggle;

    float u_MatShininess;
    float3 __Padding0;
}

cbuffer Lights : register(b3)
{
    float3 u_CameraPosition;
    int __Padding1;

    int u_SkyLightCount;
    int u_PointLightCount;
    int __Padding2;
    int __Padding3;

    PointLight u_PointLights[100];
    SkyLight u_SkyLights[10];
};

Texture2D tex : register(t0);
SamplerState sampleType : register(s0);

float3 CalculatePointLight(PointLight light, float3 normal, float3 viewDir, float3 worldPos)
{
    float3 lightDir = normalize(light.Position - worldPos);
    float diff = max(dot(normal, lightDir), 0.0);

    float3 halfwayDir = normalize(lightDir + viewDir);
    float3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_MatShininess) * light.Intensity;

    float distance = length(light.Position - worldPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distance + light.Quadratic * (distance * distance));

    float3 diffuse = light.Color * diff * u_MatColor * light.Intensity;
    float3 specular = light.Color * spec * u_MatColor;

    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular;

}

float4 main(vsOut input) : SV_TARGET
{
    float4 PixelColor;

    float3 norm = normalize(input.v_Normal);
    float3 viewDir = normalize(u_CameraPosition - input.v_WorldPos);
    float3 lightingResult = float3(1.0f, 1.0f, 1.0f);

    // Some light(s) exists in the scene, so zero the lightning result
    if (u_SkyLightCount > 0 || u_PointLightCount > 0)
        lightingResult = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < u_SkyLightCount; i++)
        lightingResult += u_SkyLights[i].Color * float3(u_SkyLights[i].Intensity, u_SkyLights[i].Intensity, u_SkyLights[i].Intensity);

    for (i = 0; i < u_PointLightCount; i++)
        lightingResult += CalculatePointLight(u_PointLights[i], norm, viewDir, input.v_WorldPos);

    if (u_MatDiffuseTexToggle == 1)
        PixelColor = tex.Sample(sampleType, input.v_TexCoord) * float4(u_MatColor, 1.0f) * float4(lightingResult, 1.0f);
    else
        PixelColor = float4(u_MatColor, 1.0f) * float4(lightingResult, 1.0f);

    return PixelColor;
}
