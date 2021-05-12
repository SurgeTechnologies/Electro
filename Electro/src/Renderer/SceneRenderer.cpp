//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Factory.hpp"
#include "Interface/Shader.hpp"
#include "Interface/ConstantBuffer.hpp"
#include "Camera/Camera.hpp"
#include "SceneRenderer.hpp"
#include "Renderer.hpp"
#include "RendererAPI.hpp"
#include "RenderCommand.hpp"

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
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> ColliderDrawList;
        Ref<Electro::EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;
    };

    static Scope<SceneCBufferData> sSceneCBufferData = CreateScope<SceneCBufferData>();
    static Scope<SceneData> sSceneData = CreateScope<SceneData>();

    void SceneRenderer::Init()
    {
        Factory::CreateShader("Electro/assets/shaders/HLSL/PBR.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Collider.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Skybox.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/EquirectangularToCubemap.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/IrradianceConvolution.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/PreFilterConvolution.hlsl");

        sSceneData->SceneCbuffer = Factory::CreateConstantBuffer(sizeof(SceneCBufferData), 0, DataUsage::DYNAMIC);
    }

    void SceneRenderer::Shutdown() {}

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
        sSceneData->ProjectionMatrix = camera.GetProjection();
        sSceneData->ViewMatrix = glm::inverse(transform);
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

    Ref<EnvironmentMap>& SceneRenderer::GetEnvironmentMapSlot()
    {
        return sSceneData->EnvironmentMap;
    }

    bool& SceneRenderer::GetEnvironmentMapActivationBool()
    {
        return sSceneData->EnvironmentMapActivated;
    }

    void SceneRenderer::EndScene()
    {
        //Upload the SceneCBufferData
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        sSceneData->SceneCbuffer->SetDynamicData(&(*sSceneCBufferData));
        sSceneData->SceneCbuffer->VSBind();

        for (auto& drawCmd : sSceneData->MeshDrawList)
            Renderer::DrawMesh(drawCmd.Mesh, drawCmd.Transform);

        for (auto& drawCmd : sSceneData->ColliderDrawList)
            Renderer::DrawColliderMesh(drawCmd.Mesh, drawCmd.Transform);

        if (sSceneData->EnvironmentMap && sSceneData->EnvironmentMapActivated)
            sSceneData->EnvironmentMap->Render(sSceneData->ProjectionMatrix, sSceneData->ViewMatrix);
    }
}