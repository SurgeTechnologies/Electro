//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroMaterial.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "Renderer/Camera/ElectroEditorCamera.hpp"
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

    //TODO: Add Directional Light
    class LightningManager
    {
    public:
        LightningManager();
        ~LightningManager() = default;

        void PushPointLight(const PointLight& pointLight);
        void PushDirectionalLight(const DirectionalLight& directionalLight);
        void CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& material);
        void ClearLights();
    private:
        Ref<ConstantBuffer> mLightConstantBuffer;
        LightCBuffer mLightCBufferData;
        Vector<PointLight> mPointLights;
        Vector<DirectionalLight> mDirectionalLights;
    };
}