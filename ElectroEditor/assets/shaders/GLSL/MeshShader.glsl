//                    SPIKE ENGINE
//Copyright 2021 - SpikeTechnologies - All Rights Reserved

#type vertex
#version 420 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    uniform mat4 u_ViewProjection;
};

layout (std140, binding = 1) uniform Mesh
{
    uniform mat4 u_Transform;
};

out VertexOutput
{
    vec3 v_Normal;
    vec2 v_TexCoord;
    vec3 v_WorldPos;
} vsOut;

void main()
{
    vsOut.v_WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
    gl_Position = u_ViewProjection * vec4(vsOut.v_WorldPos, 1.0f);
    vsOut.v_TexCoord = a_TexCoord;
    vsOut.v_Normal = a_Normal;
}

#type fragment
#version 420 core

out vec4 FragColor;

struct SkyLight
{
    vec3 Color;
    float Intensity;
};

struct PointLight
{
    vec3 Position;
    float __Padding0;

    vec3 Color;
    float __Padding1;

    float Intensity;
    float Constant;
    float Linear;
    float Quadratic;
};

in VertexOutput
{
    vec3 v_Normal;
    vec2 v_TexCoord;
    vec3 v_WorldPos;
} vsIn;

layout (std140, binding = 2) uniform Material
{
    uniform vec3  u_MatColor;
    uniform int   u_MatDiffuseTexToggle;

    uniform float u_MatShininess;
    uniform vec3 __Padding0;
};

layout (std140, binding = 3) uniform Lights
{
    uniform vec3 u_CameraPosition;
    int __Padding1;

    uniform int  u_SkyLightCount;
    uniform int  u_PointLightCount;
    int __Padding2;
    int __Padding3;

    uniform PointLight u_PointLights[100];
    uniform SkyLight u_SkyLights[10];
};

uniform sampler2D u_DiffuseTexture; //I hate this

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.Position - vsIn.v_WorldPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_MatShininess) * light.Intensity;

    float distance = length(light.Position - vsIn.v_WorldPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * distance + light.Quadratic * (distance * distance));

    vec3 diffuse = light.Color * diff * u_MatColor * light.Intensity;
    vec3 specular = light.Color * spec * u_MatColor;

    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular;
}

void main()
{
    vec3 norm = normalize(vsIn.v_Normal);
    vec3 viewDir = normalize(u_CameraPosition - vsIn.v_WorldPos);
    vec3 lightingResult = vec3(1.0, 1.0, 1.0);

    // Some light(s) exists in the scene, so zero the lightning result
    if (u_SkyLightCount > 0 || u_PointLightCount > 0)
        lightingResult = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < u_SkyLightCount; i++)
        lightingResult += u_SkyLights[i].Color * vec3(u_SkyLights[i].Intensity);

    for (int i = 0; i < u_PointLightCount; i++)
        lightingResult += CalculatePointLight(u_PointLights[i], norm, viewDir);

    if (u_MatDiffuseTexToggle == 1)
        FragColor = texture(u_DiffuseTexture, vsIn.v_TexCoord) * vec4(u_MatColor, 1.0f) * vec4(lightingResult, 1.0f);
    else
        FragColor = vec4(u_MatColor, 1.0f) * vec4(lightingResult, 1.0f);
}
