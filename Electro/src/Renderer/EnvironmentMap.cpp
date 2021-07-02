//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EnvironmentMap.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer.hpp"
#include "RenderCommand.hpp"

#include "Platform/DX11/DX11Internal.hpp"
#include <d3d11.h>

namespace Electro
{
    EnvironmentMap::EnvironmentMap(const String& hdrMapPath)
    {
        SetupAssetBase(hdrMapPath, AssetType::EnvironmentMap);
        mSkyboxShader = Renderer::GetShader("Skybox");

        mEnvironmentMap = Cubemap::Create(hdrMapPath);
        mEnvironmentMap->GenIrradianceMap();
        mEnvironmentMap->GenPreFilter();

        Texture2DSpecification textureSpec;
        textureSpec.Path = "Electro/assets/textures/BRDF_LUT.tga";
        textureSpec.Flags = TextureFlags::DEFAULT;
        mBRDFLUT = Texture2D::Create(textureSpec);

        mSkyboxCBuffer = Renderer::GetConstantBuffer(0);
        mSkyboxMaterial = Material::Create(mSkyboxShader, "SkyboxCbuffer", "SkyboxMaterial");
    }

    void EnvironmentMap::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        mSkyboxMaterial->Bind();

        mEnvironmentMap->BindIrradianceMap(5);
        mEnvironmentMap->BindPreFilterMap(6);
        mBRDFLUT->PSBindAsShaderResource(7);

        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_TextureLOD", mTextureLOD);
        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_Intensity", mIntensity);
        mSkyboxCBuffer->SetDynamicData((void*)&(projectionMatrix * glm::mat4(glm::mat3(viewMatrix))));
        mSkyboxCBuffer->VSBind();
        mSkyboxMaterial->Bind();

        mEnvironmentMap->PSBind(0);

        // https://giordi91.github.io/post/viewportclamp/
        Viewport vp = RenderCommand::GetViewport();
        RenderCommand::SetViewport({ vp.Width, vp.Height, 0.999999f, 1.0f });
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglestrip);
        RenderCommand::Draw(14);
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);
        RenderCommand::SetViewport({ vp.Width, vp.Height, 0.0f, 1.0f });

        mEnvironmentMap->Unbind(0);
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
