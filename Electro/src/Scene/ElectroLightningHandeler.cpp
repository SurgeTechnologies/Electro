//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroLightningHandeler.hpp"
#include "Core/ElectroVault.hpp"
#include "EDevice/EDevice.hpp"
#include "Renderer/ElectroRendererAPI.hpp"

namespace Electro
{
    LightningManager::LightningManager()
    {
        Ref<Shader> shader;
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: shader = Vault::Get<Shader>("MeshShader.hlsl"); break;
            case RendererAPI::API::OpenGL: shader = Vault::Get<Shader>("MeshShader.glsl"); break;
        }
        mLightConstantBuffer = EDevice::CreateConstantBuffer(sizeof(LightCBuffer), 3, DataUsage::DYNAMIC);
    }

    void LightningManager::CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& material)
    {
        material->GetShader()->Bind();

        mLightCBufferData.CameraPosition = cameraPos;
        mLightCBufferData.PointLightCount = static_cast<Uint>(mPointLights.size());

        for (int i = 0; i < mPointLights.size(); i++)
        {
            auto& light = mPointLights[i];
            mLightCBufferData.PointLights[i].Position  = light.Position;
            mLightCBufferData.PointLights[i].Intensity = light.Intensity;
            mLightCBufferData.PointLights[i].Color     = light.Color;
        }

        mLightConstantBuffer->SetDynamicData(&mLightCBufferData);
        mLightConstantBuffer->PSBind();
    }

    void LightningManager::ClearLights()
    {
        mPointLights.clear();
    }

    void LightningManager::PushPointLight(PointLight& pointLight)
    {
        mPointLights.emplace_back(pointLight);
    }
}
