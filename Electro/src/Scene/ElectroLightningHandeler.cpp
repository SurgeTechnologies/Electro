//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroLightningHandeler.hpp"
#include "Core/ElectroVault.hpp"
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

        //Setup the lights constant buffer
        ConstantBufferDesc desc = {};
        desc.Shader = shader;
        desc.Name = "Lights";
        desc.InitialData = nullptr;
        desc.Size = sizeof(LightCBuffer);
        desc.BindSlot = 3;
        desc.ShaderDomain = ShaderDomain::PIXEL;
        desc.Usage = DataUsage::DYNAMIC;
        mLightConstantBuffer = ConstantBuffer::Create(desc);
    }

    LightningManager::~LightningManager() {}

    void LightningManager::CalculateAndRenderLights(const glm::vec3& cameraPos, Ref<Material>& material)
    {
        Ref<Shader>& shader = material->GetShader();
        shader->Bind();

        mLightCBufferData.CameraPosition = cameraPos;
        mLightCBufferData.SkyLightCount = static_cast<Uint>(mSkyLights.size());
        mLightCBufferData.PointLightCount = static_cast<Uint>(mPointLights.size());

        for (int i = 0; i < mPointLights.size(); i++)
        {
            auto& light = mPointLights[i];
            mLightCBufferData.PointLights[i].Position  = light.Position;
            mLightCBufferData.PointLights[i].Color     = light.Color;
            mLightCBufferData.PointLights[i].Intensity = light.Intensity;
            mLightCBufferData.PointLights[i].Constant  = light.Constant;
            mLightCBufferData.PointLights[i].Quadratic = light.Quadratic;
            mLightCBufferData.PointLights[i].Linear    = light.Linear;
        }

        for (int i = 0; i < mSkyLights.size(); i++)
        {
            auto& light = mSkyLights[i];
            mLightCBufferData.SkyLights[i].Direction = light.Direction;
            mLightCBufferData.SkyLights[i].Intensity = light.Intensity;
            mLightCBufferData.SkyLights[i].Color = light.Color;
        }

        mLightConstantBuffer->SetData(&mLightCBufferData);
    }

    void LightningManager::ClearLights()
    {
        mSkyLights.clear();
        mPointLights.clear();
    }

    void LightningManager::PushSkyLight(SkyLight& skyLight)
    {
        mSkyLights.emplace_back(skyLight);
    }

    void LightningManager::PushPointLight(PointLight& pointLight)
    {
        mPointLights.emplace_back(pointLight);
    }
}