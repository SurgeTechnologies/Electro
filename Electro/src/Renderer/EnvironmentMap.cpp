//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EnvironmentMap.hpp"
#include "Asset/AssetManager.hpp"
#include "Generator.hpp"
#include "Interface/VertexBuffer.hpp"
#include "Interface/IndexBuffer.hpp"
#include "Interface/Framebuffer.hpp"
#include "RenderCommand.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Electro
{
    EnvironmentMap::EnvironmentMap(const String& hdrMapPath)
    {
        mPBRShader = AssetManager::Get<Shader>("PBR.hlsl");
        mEnvironmentMap = EGenerator::CreateCubemap(hdrMapPath);
        mEnvironmentMap->GenIrradianceMap();
        mEnvironmentMap->GenPreFilter();
        mBRDFLUT = EGenerator::CreateTexture2D("Electro/assets/textures/BRDF_LUT.tga");

        mSkyboxCBuffer = EGenerator::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);

        mSkyboxShader = AssetManager::Get<Shader>("Skybox.hlsl");
        mSkyboxMaterial = EGenerator::CreateMaterial(mSkyboxShader, "SkyboxCbuffer", "Skybox");
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

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLESTRIP);
        RenderCommand::Draw(14);
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

        RenderCommand::SetDepthTest(DepthTestFunc::Less);
    }
}
