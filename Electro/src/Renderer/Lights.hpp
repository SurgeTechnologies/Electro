//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>

namespace Electro
{
    struct SkyLight
    {
        glm::vec3 Direction;
        float __Padding0;

        glm::vec3 Color;
        float Intensity;
    };

    struct PointLight
    {
        glm::vec3 Position;
        float Intensity;
        glm::vec3 Color;
        float Radius;
    };

    struct DirectionalLight
    {
        glm::vec3 Direction;
        float Intensity;
        glm::vec3 Color;
        float __Padding0;
    };

    struct LightCBuffer
    {
        glm::vec3 CameraPosition;
        float __Padding0;

        int PointLightCount;
        int DirectionalLightCount;
        glm::vec2 __Padding1;

        PointLight PointLights[100];
        DirectionalLight DirectionalLights[4];
    };
}
