//                    SPIKE ENGINE
//Copyright 2021 - SpikeTechnologies - All Rights Reserved

#type vertex
#version 430 core
layout(location = 0) in vec3 SKYBOX_POS;

layout (std140, binding = 0) uniform SkyboxCBuffer { uniform mat4 u_ViewProjection; };
out vec3 v_TexCoords;

void main()
{
    v_TexCoords = SKYBOX_POS;
    vec4 pos = u_ViewProjection * vec4(SKYBOX_POS, 1.0);
    gl_Position = pos.xyww;
}


#type fragment
#version 430 core

out vec4 FragColor;
in vec3 v_TexCoords;
uniform samplerCube u_SkyboxCubemap;

void main()
{
    FragColor = texture(u_SkyboxCubemap, v_TexCoords);
}