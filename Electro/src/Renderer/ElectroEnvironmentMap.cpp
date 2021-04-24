//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroEnvironmentMap.hpp"
#include "Core/ElectroVault.hpp"
#include "EDevice/EDevice.hpp"
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
        float vertices[] =
        {
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f
        };

        Uint indices[] =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        mEnvironmentMap = Cubemap::Create(hdrMapPath);
        mEnvironmentMap->GenIrradianceMap();
        mEnvironmentMap->GenPreFilter();
        mBRDFLUT = Texture2D::Create("Electro/assets/textures/BRDF_LUT.tga");

        Vault::Get<Shader>("Skybox.hlsl")->Bind();
        mSkyboxCBuffer = ConstantBuffer::Create(sizeof(glm::mat4), 0);

        //Pipeline for the skybox
        VertexBufferLayout layout = { { ShaderDataType::Float3, "SKYBOX_POS" } };
        Ref<VertexBuffer> vertexBuffer = EDevice::CreateVertexBuffer(vertices, sizeof(vertices), layout);
        Ref<IndexBuffer> indexBuffer   = EDevice::CreateIndexBuffer(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification spec;
        spec.VertexBuffer = vertexBuffer;
        spec.IndexBuffer = indexBuffer;
        spec.Shader = Vault::Get<Shader>("Skybox.hlsl");
        mPipeline = EDevice::CreatePipeline(spec);
    }

    void EnvironmentMap::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        RenderCommand::SetDepthTest(DepthTestFunc::LEqual);

        auto deviceContext = DX11Internal::GetDeviceContext();

        Vault::Get<Shader>("PBR.hlsl")->Bind();
        mEnvironmentMap->BindIrradianceMap(5);
        mEnvironmentMap->BindPreFilterMap(6);
        mBRDFLUT->Bind(7);

        mPipeline->Bind();
        mPipeline->BindSpecificationObjects();
        mSkyboxCBuffer->SetData((void*)&(projectionMatrix * glm::mat4(glm::mat3(viewMatrix))));
        mSkyboxCBuffer->Bind();
        mEnvironmentMap->Bind(32);
        RenderCommand::DrawIndexed(mPipeline, 36);

        RenderCommand::SetDepthTest(DepthTestFunc::Less);
    }
}
