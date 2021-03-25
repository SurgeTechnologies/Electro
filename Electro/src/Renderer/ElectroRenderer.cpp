//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroSkybox.hpp"

namespace Electro
{
    struct SceneCBufferData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    struct DrawCommand
    {
        Ref<Mesh> TheRealMesh;
        glm::mat4 Transform;
    };

    struct SceneData
    {
        glm::mat4 ProjectionMatrix, ViewMatrix;
        Ref<ConstantBuffer> SceneCbuffer;
        size_t DrawCalls = 0;
        Ref<Electro::Skybox> Skybox;
        bool SkyboxActivated = true;
        Vector<DrawCommand> DrawCommands;
    };

    static Scope<SceneCBufferData> sSceneCBufferData = CreateScope<SceneCBufferData>();
    static Scope<SceneData> sSceneData = CreateScope<SceneData>();

    void Renderer::Init()
    {
        RenderCommand::Init();
        Ref<Shader> shader;

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: shader =   Shader::Create("Electro/assets/shaders/HLSL/MeshShader.hlsl"); break;
            case RendererAPI::API::OpenGL: shader = Shader::Create("Electro/assets/shaders/GLSL/MeshShader.glsl"); break;
        }

        Vault::Submit<Shader>(shader);

        ConstantBufferDesc desc;
        desc.Shader = shader;
        desc.Name = "Camera";
        desc.InitialData = nullptr;
        desc.Size = sizeof(SceneCBufferData);
        desc.BindSlot = 0;
        desc.ShaderDomain = ShaderDomain::VERTEX;
        desc.Usage = DataUsage::DYNAMIC;
        sSceneData->SceneCbuffer = ConstantBuffer::Create(desc);
    }

    void Renderer::Shutdown()
    {
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        sSceneCBufferData->ViewProjectionMatrix = camera.GetViewProjection();
        sSceneData->ProjectionMatrix = camera.GetProjection();
        sSceneData->ViewMatrix = camera.GetViewMatrix();
        sSceneData->DrawCommands.clear();
    }

    void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        sSceneCBufferData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sSceneData->DrawCommands.clear();
    }

    void Renderer::EndScene()
    {
        for (auto& drawCmd : sSceneData->DrawCommands)
        {
            auto& mesh = drawCmd.TheRealMesh;
            auto& transform = drawCmd.Transform;

            mesh->GetPipeline()->Bind();
            mesh->GetPipeline()->BindSpecificationObjects();

            sSceneData->SceneCbuffer->SetData(&(*sSceneCBufferData)); //Upload the SceneCBufferData

            for (Submesh& submesh : mesh->GetSubmeshes())
            {
                mesh->GetMaterial()->Bind(submesh.MaterialIndex);
                submesh.CBuffer->SetData(&(transform * submesh.Transform));
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                sSceneData->DrawCalls++;
            }
        }

        if (sSceneData->Skybox && sSceneData->SkyboxActivated)
            sSceneData->Skybox->Render(sSceneData->ProjectionMatrix, sSceneData->ViewMatrix);
    }

    void Renderer::Submit(Ref<Pipeline> pipeline, Uint size)
    {
        RenderCommand::DrawIndexed(pipeline, size);
    }

    void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        sSceneData->DrawCommands.push_back({ mesh, transform });
    }

    void Renderer::UpdateStats()
    {
        sSceneData->DrawCalls = 0;
    }

    RendererAPI::API Renderer::GetAPI()
    {
        return RendererAPI::GetAPI();
    }

    size_t Renderer::GetTotalDrawCallsCount()
    {
        return sSceneData->DrawCalls;
    }

    void Renderer::SetSkyboxActivationBool(bool vaule)
    {
        sSceneData->SkyboxActivated = vaule;
    }

    Ref<Skybox>& Renderer::SetSkybox(const Ref<Skybox>& skybox)
    {
        sSceneData->Skybox = skybox;
        return sSceneData->Skybox;
    }

    bool& Renderer::GetSkyboxActivationBool()
    {
        return sSceneData->SkyboxActivated;
    }
}