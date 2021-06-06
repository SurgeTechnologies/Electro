//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "LightningHandeler.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Renderer.hpp"

namespace Electro
{
    LightningManager::LightningManager()
    {
        mLightConstantBuffer = Renderer::GetConstantBuffer(3);
    }

    void LightningManager::CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& mat)
    {
        mat->GetShader()->Bind();

        mLightCBufferData.CameraPosition = cameraPos;
        mLightCBufferData.PointLightCount = static_cast<Uint>(mPointLights.size());
        mLightCBufferData.DirectionalLightCount = static_cast<Uint>(mDirectionalLights.size());

        for (int i = 0; i < mPointLights.size(); i++)
        {
            auto& light = mPointLights[i];
            mLightCBufferData.PointLights[i].Position  = light.Position;
            mLightCBufferData.PointLights[i].Intensity = light.Intensity;
            mLightCBufferData.PointLights[i].Color     = light.Color;
        }

        for (int i = 0; i < mDirectionalLights.size(); i++)
        {
            auto& light = mDirectionalLights[i];
            mLightCBufferData.DirectionalLights[i].Direction = light.Direction;
            mLightCBufferData.DirectionalLights[i].Intensity = light.Intensity;
            mLightCBufferData.DirectionalLights[i].Color     = light.Color;
        }

        mLightConstantBuffer->SetDynamicData(&mLightCBufferData);
        mLightConstantBuffer->PSBind();
    }

    void LightningManager::ClearLights()
    {
        mPointLights.clear();
        mDirectionalLights.clear();
    }

    void LightningManager::PushPointLight(const PointLight& pointLight)
    {
        mPointLights.emplace_back(pointLight);
    }

    void LightningManager::PushDirectionalLight(const DirectionalLight& directionalLight)
    {
        mDirectionalLights.emplace_back(directionalLight);
    }
}
