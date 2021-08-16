//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EnvironmentMap.hpp"
#include "Renderer.hpp"
#include "RenderCommand.hpp"


namespace Electro
{
    EnvironmentMap::EnvironmentMap(const String& hdrMapPath)
        : mPath(hdrMapPath)
    {
        mSkyboxShader = Renderer::GetShader("Skybox");

        mEnvironmentMap = Cubemap::Create(hdrMapPath);

        mEnvironmentMap->GenIrradianceMap();
        mEnvironmentMap->GenPreFilter();

        Texture2DSpecification spec;
        spec.Path = "Electro/Assets/Renderer/BRDF_LUT.tga";
        mBRDFLUT = Texture2D::Create(spec);

        mSkyboxCBuffer = Renderer::GetConstantBuffer(0);
        mSkyboxMaterial = Material::Create(mSkyboxShader, "SkyboxCbuffer", "SkyboxMaterial");
    }

    void EnvironmentMap::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        mSkyboxMaterial->Bind();

        mEnvironmentMap->BindIrradianceMap(5);
        mEnvironmentMap->BindPreFilterMap(6);
        mBRDFLUT->Bind(7, ShaderDomain::PIXEL);

        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_TextureLOD", mTextureLOD);
        mSkyboxMaterial->Set<float>("SkyboxCbuffer.u_Intensity", mIntensity);

        glm::mat4 skyboxData = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));
        mSkyboxCBuffer->SetDynamicData(&skyboxData);

        mSkyboxCBuffer->VSBind();
        mSkyboxMaterial->Bind();

        mEnvironmentMap->PSBind(0);

        // https://giordi91.github.io/post/viewportclamp/
        Viewport vp = RenderCommand::GetViewport();
        RenderCommand::SetViewport({ vp.Width, vp.Height, 0.999999f, 1.0f });
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLESTRIP);
        RenderCommand::Draw(14);
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        RenderCommand::SetViewport({ vp.Width, vp.Height, 0.0f, 1.0f });

        mEnvironmentMap->Unbind(0);
    }

    Ref<EnvironmentMap> EnvironmentMap::Create(const String& path)
    {
        return Ref<EnvironmentMap>::Create(path);
    }
}
