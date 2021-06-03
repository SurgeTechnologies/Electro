//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "SceneRenderer.hpp"
#include "Renderer.hpp"
#include "EditorModule.hpp"
#include <imgui_internal.h>

namespace Electro
{
    static Scope<SceneRendererData> sData = CreateScope<SceneRendererData>();

    void SceneRenderer::Init()
    {
        // Creates and submits all the shader to AssetManager
        Factory::CreateShader("Electro/assets/shaders/HLSL/PBR.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Collider.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Skybox.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/EquirectangularToCubemap.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/IrradianceConvolution.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/PreFilterConvolution.hlsl");
        sData->ShadowMapShader = Factory::CreateShader("Electro/assets/shaders/HLSL/ShadowMap.hlsl");

        sData->ShadowMapCascades.Init();

        // Cascade matrices size is NUM_CASCADES and '+ 1' is for view matrix
        sData->LightSpaceMatrixCBuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4) * (NUM_CASCADES + 1), 6, DataUsage::DYNAMIC);
        sData->CascadeEndsCBuffer = Factory::CreateConstantBuffer(sizeof(glm::vec4), 7, DataUsage::DYNAMIC);
        sData->SceneCBuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
    }

    void SceneRenderer::Shutdown() {}

    void SceneRenderer::BeginScene(EditorCamera& camera)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to SceneRenderer::BeginScene() without matching SceneRenderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetViewProjection();
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = camera.GetViewMatrix();
    }

    void SceneRenderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to SceneRenderer::BeginScene() without matching SceneRenderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = glm::inverse(transform);
    }

    void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        sData->MeshDrawList.push_back({ mesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform)
    {
        sData->ColliderDrawList.push_back({ component.DebugMesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform)
    {
        sData->ColliderDrawList.push_back({ component.DebugMesh, transform });
    }

    void SceneRenderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform)
    {
        for (const Ref<Mesh>& debugMesh : component.ProcessedMeshes)
            sData->ColliderDrawList.push_back({ debugMesh, transform });
    }

    void SceneRenderer::ShadowPass()
    {
        glm::vec3 direction;
        if (sData->SceneContext)
        {
            auto view = sData->SceneContext->mRegistry.view<TransformComponent, DirectionalLightComponent>();
            for (const entt::entity& entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                direction = transform.GetTransform()[2]; //Z axis of rotation matrix
            }
        }

        // Calculate the ViewProjection matrices
        sData->ShadowMapCascades.CalculateViewProjection(sData->ViewMatrix, sData->ProjectionMatrix, glm::normalize(direction));
        sData->CascadeEndsCBuffer->SetDynamicData(&sData->ShadowMapCascades.GetCascadeSplitDepths());
        sData->CascadeEndsCBuffer->PSBind();

        RenderCommand::SetCullMode(CullMode::Front);
        // Loop over all the shadow maps and bind and render the whole scene to each of them
        for (Uint j = 0; j < NUM_CASCADES; j++)
        {
            const Ref<Framebuffer>& shadowMapBuffer = sData->ShadowMapCascades.GetFramebuffers()[j];
            shadowMapBuffer->Bind();
            sData->ShadowMapShader->Bind();
            shadowMapBuffer->Clear();

            // Set the LightSpaceMatrix
            sData->SceneCBuffer->SetDynamicData((void*)(&sData->ShadowMapCascades.GetViewProjections()[j]));
            sData->SceneCBuffer->VSBind();

            for (const DrawCommand& drawCmd : sData->MeshDrawList)
            {
                const Ref<Mesh>& mesh = drawCmd.Mesh;
                const PipelineSpecification& spec = mesh->GetPipeline()->GetSpecification();
                const Submesh* submeshes = mesh->GetSubmeshes().data();

                mesh->GetPipeline()->Bind();
                spec.VertexBuffer->Bind();
                spec.IndexBuffer->Bind();

                for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
                {
                    const Submesh& submesh = submeshes[i];
                    submesh.CBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                    submesh.CBuffer->VSBind();
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }
        }
        RenderCommand::SetCullMode(CullMode::None);
    }

    void SceneRenderer::GeometryPass()
    {
        if (!sData->ActiveRenderBuffer)
            return;

        sData->ActiveRenderBuffer->Bind();

        sData->SceneCBuffer->SetDynamicData(&sData->ViewProjectionMatrix);
        sData->SceneCBuffer->VSBind();

        // Loop over the total number of cascades and set the light ViewProjection
        glm::mat4 lightMatData[NUM_CASCADES + 1];
        for (Uint i = 0; i < NUM_CASCADES; i++)
            lightMatData[i] = sData->ShadowMapCascades.GetViewProjections()[i];

        // The view matrix
        lightMatData[NUM_CASCADES] = sData->ViewMatrix;

        // Set the LightMatrix Data to the Vertex shader
        sData->LightSpaceMatrixCBuffer->SetDynamicData(lightMatData);
        sData->LightSpaceMatrixCBuffer->VSBind();

        // Bind the shadow maps(which was captured from the ShadowPass()) as texture and draw all the objects in the scene
        // NOTE: Here starting slot is 8, so the shadow maps gets bound as 8, 9, 10, ..., n
        sData->ShadowMapCascades.Bind(8);
        for (const DrawCommand& drawCmd : sData->MeshDrawList)
            Renderer::DrawMesh(drawCmd.Mesh, drawCmd.Transform);
        sData->ShadowMapCascades.Unbind(8);

        // Render all the colliders
        for (const DrawCommand& drawCmd : sData->ColliderDrawList)
            Renderer::DrawColliderMesh(drawCmd.Mesh, drawCmd.Transform);

        sData->ActiveRenderBuffer->Unbind();
    }

    void SceneRenderer::EndScene()
    {
        ShadowPass();
        GeometryPass();

        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);

        ClearDrawList();
    }

    bool SceneRenderer::IsDrawListEmpty() { return (sData->MeshDrawList.empty() && sData->ColliderDrawList.empty()); }
    void SceneRenderer::ClearDrawList()
    {
        sData->MeshDrawList.clear();
        sData->ColliderDrawList.clear();
    }

    Ref<EnvironmentMap>& SceneRenderer::GetEnvironmentMapSlot()
    {
        return sData->EnvironmentMap;
    }

    bool& SceneRenderer::GetEnvironmentMapActivationBool()
    {
        return sData->EnvironmentMapActivated;
    }

    void SceneRenderer::SetSceneContext(Scene* sceneContext)
    {
        sData->SceneContext = sceneContext;
    }

    void SceneRenderer::SetActiveRenderBuffer(Ref<Framebuffer>& renderBuffer)
    {
        sData->ActiveRenderBuffer = renderBuffer;
    }

    const Scope<SceneRendererData>& SceneRenderer::GetData()
    {
        return sData;
    }
}
