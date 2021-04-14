//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroShader.hpp"
#include "ElectroSceneRenderer.hpp"
#include "ElectroConstantBuffer.hpp"
#include "ElectroRenderer.hpp"
#include "ElectroRendererAPI.hpp"
#include "ElectroRenderCommand.hpp"

namespace Electro
{
    struct SceneCBufferData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    struct DrawCommand
    {
        Ref<Electro::Mesh> Mesh;
        glm::mat4 Transform;
    };

    struct SceneData
    {
        glm::mat4 ProjectionMatrix, ViewMatrix;
        Ref<ConstantBuffer> SceneCbuffer;
        size_t DrawCalls = 0;
        Ref<Electro::Skybox> Skybox;
        bool SkyboxActivated = true;
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> ColliderDrawList;
    };

    static Scope<SceneCBufferData> sSceneCBufferData = CreateScope<SceneCBufferData>();
    static Scope<SceneData> sSceneData = CreateScope<SceneData>();

    void SceneRenderer::Init()
    {
        Ref<Shader> shader;
        Ref<Shader> colliderShader;
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                shader         = Shader::Create("Electro/assets/shaders/HLSL/PBR.hlsl");
                colliderShader = Shader::Create("Electro/assets/shaders/HLSL/Collider.hlsl"); break;
            case RendererAPI::API::OpenGL:
                shader         = Shader::Create("Electro/assets/shaders/GLSL/PBR.glsl");
                colliderShader = Shader::Create("Electro/assets/shaders/GLSL/Collider.glsl"); break;
        }

        Vault::Submit<Shader>(shader);
        Vault::Submit<Shader>(colliderShader);

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

    void SceneRenderer::Shutdown()
    {
    }

    void SceneRenderer::BeginScene(EditorCamera& camera)
    {
        sSceneCBufferData->ViewProjectionMatrix = camera.GetViewProjection();
        sSceneData->ProjectionMatrix = camera.GetProjection();
        sSceneData->ViewMatrix = camera.GetViewMatrix();
        sSceneData->MeshDrawList.clear();
        sSceneData->ColliderDrawList.clear();
    }

    void SceneRenderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        sSceneCBufferData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sSceneData->MeshDrawList.clear();
        sSceneData->ColliderDrawList.clear();
    }

    void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        sSceneData->MeshDrawList.push_back({ mesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform)
    {
        sSceneData->ColliderDrawList.push_back({ component.DebugMesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform)
    {
        sSceneData->ColliderDrawList.push_back({ component.DebugMesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform)
    {
        for (auto& debugMesh : component.ProcessedMeshes)
            sSceneData->ColliderDrawList.push_back({ debugMesh, transform });
    }

    void SceneRenderer::EndScene()
    {
        //Upload the SceneCBufferData
        sSceneData->SceneCbuffer->SetData(&(*sSceneCBufferData));

        for (auto& drawCmd : sSceneData->MeshDrawList)
            Renderer::DrawMesh(drawCmd.Mesh, drawCmd.Transform);

        for (auto& drawCmd : sSceneData->ColliderDrawList)
            Renderer::DrawColliderMesh(drawCmd.Mesh, drawCmd.Transform);

        // Render the skybox at the last
        if (sSceneData->Skybox && sSceneData->SkyboxActivated)
            sSceneData->Skybox->Render(sSceneData->ProjectionMatrix, sSceneData->ViewMatrix);
    }

    void SceneRenderer::SetSkyboxActivationBool(bool vaule)
    {
        sSceneData->SkyboxActivated = vaule;
    }

    Ref<Skybox>& SceneRenderer::SetSkybox(const Ref<Skybox>& skybox)
    {
        sSceneData->Skybox = skybox;
        return sSceneData->Skybox;
    }

    bool& SceneRenderer::GetSkyboxActivationBool()
    {
        return sSceneData->SkyboxActivated;
    }
}