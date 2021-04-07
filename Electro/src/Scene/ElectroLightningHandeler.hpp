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
        glm::vec3 Color;
        float Intensity;
    };

    struct PointLight
    {
        glm::vec3 Position;
        int __Padding0;

        glm::vec3 Color;
        float __Padding1;

        float Intensity;
        float Constant;
        float Linear;
        float Quadratic;
    };

    struct LightCBuffer
    {
        glm::vec3 CameraPosition;
        float __Padding0;

        int SkyLightCount;
        int PointLightCount;
        int __Padding1;
        int __Padding2;

        PointLight PointLights[100];
        SkyLight AmbientLights[10];
    };

    class LightningManager
    {
    public:
        LightningManager();
        ~LightningManager();

        void PushSkyLight(SkyLight& skyLight);
        void PushPointLight(PointLight& pointLight);
        void CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& material);
        void ClearLights();

    private:
        Ref<ConstantBuffer> mLightConstantBuffer;
        LightCBuffer mLightCBufferData;
        Vector<SkyLight> mSkyLights;
        Vector<PointLight> mPointLights;
    };
}