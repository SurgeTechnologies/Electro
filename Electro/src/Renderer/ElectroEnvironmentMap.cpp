//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroEnvironmentMap.hpp"
#include "Asset/ElectroAssetManager.hpp"
#include "EGenerator.hpp"
#include "Interface/ElectroVertexBuffer.hpp"
#include "Interface/ElectroIndexBuffer.hpp"
#include "Interface/ElectroFramebuffer.hpp"
#include "ElectroRenderCommand.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Platform/DX11/DX11Internal.hpp"

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
        auto deviceContext = DX11Internal::GetDeviceContext();

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
