//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EnvironmentMap.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer.hpp"
#include "RenderCommand.hpp"


namespace Electro
{
    EnvironmentMap::EnvironmentMap(const String& hdrMapPath)
    {
        SetupAssetBase(hdrMapPath, AssetType::EnvironmentMap);
        mPBRShader    = Renderer::GetShader("PBR");
        mSkyboxShader = Renderer::GetShader("Skybox");

        mEnvironmentMap = Cubemap::Create(hdrMapPath);
        mEnvironmentMap->GenIrradianceMap();
        mEnvironmentMap->GenPreFilter();
        mBRDFLUT = Texture2D::Create("Electro/assets/textures/BRDF_LUT.tga");

        mSkyboxCBuffer = ConstantBuffer::Create(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);

        mSkyboxMaterial = Material::Create(mSkyboxShader, "SkyboxCbuffer", "SkyboxMaterial");
    }

    void EnvironmentMap::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        RenderCommand::SetDepthTest(DepthTestFunc::LEqual);

        mPBRShader->Bind();
        mEnvironmentMap->BindIrradianceMap(5);
        mEnvironmentMap->BindPreFilterMap(6);
        mBRDFLUT->PSBind(7);

        mSkyboxCBuffer->SetDynamicData((void*)&(projectionMatrix * glm::mat4(glm::mat3(viewMatrix))));
        mSkyboxCBuffer->VSBind();

        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_TextureLOD", mTextureLOD);
        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_Intensity", mIntensity);
        mSkyboxMaterial->Bind();
        mEnvironmentMap->PSBind(32);
        mSkyboxShader->Bind();

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglestrip);
        RenderCommand::Draw(14);
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

        RenderCommand::SetDepthTest(DepthTestFunc::Less);
    }

    Ref<EnvironmentMap> EnvironmentMap::Create(const String& path)
    {
        Ref<EnvironmentMap> result = nullptr;
        if (!AssetManager::Exists(path))
        {
            result = Ref<EnvironmentMap>::Create(path);
            AssetManager::Submit<EnvironmentMap>(result);
        }
        else
            result = AssetManager::Get<EnvironmentMap>(AssetManager::GetHandle(path));

        return result;
    }
}
