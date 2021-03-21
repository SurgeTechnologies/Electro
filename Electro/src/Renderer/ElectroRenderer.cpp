//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroSkybox.hpp"

namespace Electro::Renderer
{
    struct SceneCBufferData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    struct SceneData
    {
        glm::mat4 ProjectionMatrix, ViewMatrix;
        Ref<ConstantBuffer> SceneCbuffer;
        size_t DrawCalls = 0;
        Ref<Electro::Skybox> Skybox;
        bool SkyboxActivated = true;
    };

    Scope<SceneCBufferData> sceneCBufferData = CreateScope<SceneCBufferData>();
    Scope<SceneData> sceneData = CreateScope<SceneData>();

    void Init()
    {
        RenderCommand::Init();
        Ref<Shader> shader;

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: shader =   Shader::Create("Electro/assets/shaders/HLSL/MeshShader.hlsl"); break;
            case RendererAPI::API::OpenGL: shader = Shader::Create("Electro/assets/shaders/GLSL/MeshShader.glsl"); break;
        }

        Vault::Submit<Shader>(shader);
        sceneData->SceneCbuffer = ConstantBuffer::Create(shader, "Camera", nullptr, sizeof(SceneCBufferData), 0, ShaderDomain::VERTEX, DataUsage::DYNAMIC);
    }

    void Shutdown()
    {
    }

    void OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void BeginScene(EditorCamera& camera)
    {
        sceneCBufferData->ViewProjectionMatrix = camera.GetViewProjection();
        sceneData->ProjectionMatrix = camera.GetProjection();
        sceneData->ViewMatrix = camera.GetViewMatrix();
    }

    void BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        sceneCBufferData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
    }

    void EndScene()
    {
        if (sceneData->Skybox && sceneData->SkyboxActivated)
            sceneData->Skybox->Render(sceneData->ProjectionMatrix, sceneData->ViewMatrix);
    }

    void Submit(Ref<Pipeline> pipeline, Uint size)
    {
        RenderCommand::DrawIndexed(pipeline, size);
    }

    void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        auto shader = mesh->GetShader();
        shader->Bind();
        mesh->GetVertexBuffer()->Bind();
        mesh->GetPipeline()->Bind();
        mesh->GetIndexBuffer()->Bind();

        sceneData->SceneCbuffer->SetData(&(*sceneCBufferData)); //Upload the sceneCBufferData
        for (Submesh& submesh : mesh->GetSubmeshes())
        {
            mesh->GetMaterial()->Bind(submesh.MaterialIndex);
            submesh.CBuffer->SetData(&(transform * submesh.Transform));
            RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            sceneData->DrawCalls++;
        }
    }

    void UpdateStats()
    {
        sceneData->DrawCalls = 0;
    }

    RendererAPI::API GetAPI()
    {
        return RendererAPI::GetAPI();
    }

    size_t GetTotalDrawCallsCount()
    {
        return sceneData->DrawCalls;
    }

    void SetSkyboxActivationBool(bool vaule)
    {
        sceneData->SkyboxActivated = vaule;
    }

    Ref<Skybox>& SetSkybox(const Ref<Skybox>& skybox)
    {
        sceneData->Skybox = skybox;
        return sceneData->Skybox;
    }
}