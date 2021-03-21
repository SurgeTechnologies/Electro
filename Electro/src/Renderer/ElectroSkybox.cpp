//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroSkybox.hpp"
#include "ElectroVertexBuffer.hpp"
#include "ElectroIndexBuffer.hpp"
#include "ElectroRenderCommand.hpp"

namespace Electro
{
    Skybox::Skybox(const Ref<TextureCube>& texture)
        :mTexture(texture)
    {
        float skyboxVertices[] =
        {
            // positions
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

        VertexBufferLayout layout = { { ShaderDataType::Float3, "SKYBOX_POS" } };

        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices), layout);
        Ref<IndexBuffer> indexBuffer= IndexBuffer::Create(indices, std::size(indices));

        Ref<Shader> skyboxShader;
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: skyboxShader = Shader::Create("Electro/assets/shaders/HLSL/Skybox.hlsl"); break;
            case RendererAPI::API::OpenGL: skyboxShader = Shader::Create("Electro/assets/shaders/GLSL/Skybox.glsl"); break;
        }
        skyboxShader->Bind();
        //skyboxShader->SetInt("u_SkyboxCubemap", 32); //I hate OpenGL

        mSkyboxCBuffer = ConstantBuffer::Create(skyboxShader, "SkyboxCBuffer", nullptr, sizeof(glm::mat4), 0, ShaderDomain::VERTEX, DataUsage::DYNAMIC);
        PipelineSpecification spec;
        spec.VertexBuffer = vertexBuffer;
        spec.IndexBuffer = indexBuffer;
        spec.Shader = skyboxShader;
        mPipeline = Pipeline::Create(spec);
    }

    void Skybox::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        RenderCommand::SetDepthTest(DepthTestFunc::LEqual);

        mPipeline->Bind();
        mPipeline->BindSpecificationObjects();
        mTexture->Bind(32, ShaderDomain::PIXEL);
        mSkyboxCBuffer->SetData((void*)&(projectionMatrix * glm::mat4(glm::mat3(viewMatrix))));
        RenderCommand::DrawIndexed(mPipeline, 36);

        RenderCommand::SetDepthTest(DepthTestFunc::Less);
    }

    Ref<Skybox> Skybox::Create(const Ref<TextureCube>& texture)
    {
        return Ref<Skybox>::Create(texture);
    }
}