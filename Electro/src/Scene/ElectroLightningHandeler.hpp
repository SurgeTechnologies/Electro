//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroMaterial.hpp"
#include "Renderer/ElectroEditorCamera.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
#include "Scene/ElectroComponents.hpp"
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
        float __Padding0;
    };

    struct LightCBuffer
    {
        glm::vec3 CameraPosition;
        float __Padding0;

        int PointLightCount;
        glm::vec3 __Padding1;

        PointLight PointLights[100];
    };

    class LightningManager
    {
    public:
        LightningManager();
        ~LightningManager();

        void PushPointLight(PointLight& pointLight);
        void CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& material);
        void ClearLights();

    private:
        Ref<ConstantBuffer> mLightConstantBuffer;
        LightCBuffer mLightCBufferData;
        Vector<PointLight> mPointLights;
    };
}