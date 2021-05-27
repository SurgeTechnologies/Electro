//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Factory.hpp"
#include "Interface/ConstantBuffer.hpp"
#include "Interface/Framebuffer.hpp"
#include "Camera/Camera.hpp"
#include "SceneRenderer.hpp"
#include "Renderer.hpp"
#include "RendererAPI.hpp"
#include "Cascades.hpp"
#include "RenderCommand.hpp"
#include "EditorModule.hpp"
#include "RendererDebug.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"

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
        EditorModule* Context;
        Scene* SceneContext;
        glm::mat4 ProjectionMatrix, ViewMatrix;
        Ref<ConstantBuffer> SceneCBuffer;
        Ref<ConstantBuffer> LightSpaceMatrixCBuffer;
        Ref<ConstantBuffer> CascadeEndsCBuffer;
        size_t DrawCalls = 0;
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> ColliderDrawList;

        //Environment Map
        Ref<EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        //Shadows
        Ref<Shader> ShadowMapShader;
        Cascades ShadowMapCascades;
        //Temp
        EditorCamera Camera;
    };

    static Scope<SceneCBufferData> sSceneCBufferData = CreateScope<SceneCBufferData>();
    static Scope<SceneData> sData = CreateScope<SceneData>();

    void SceneRenderer::Init()
    {
        Factory::CreateShader("Electro/assets/shaders/HLSL/PBR.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Collider.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Skybox.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/EquirectangularToCubemap.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/IrradianceConvolution.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/PreFilterConvolution.hlsl");
        sData->ShadowMapShader = Factory::CreateShader("Electro/assets/shaders/HLSL/ShadowMap.hlsl");

        sData->ShadowMapCascades.Init();
        sData->SceneCBuffer = Factory::CreateConstantBuffer(sizeof(SceneCBufferData), 0, DataUsage::DYNAMIC);
        sData->CascadeEndsCBuffer = Factory::CreateConstantBuffer(sizeof(float) * (NUM_CASCADES + 1), 7, DataUsage::DYNAMIC);
        sData->LightSpaceMatrixCBuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4) * NUM_CASCADES, 6, DataUsage::DYNAMIC);
    }

    void SceneRenderer::Shutdown() {}

    void SceneRenderer::BeginScene(EditorCamera& camera)
    {
        sData->Camera = camera;
        sSceneCBufferData->ViewProjectionMatrix = camera.GetViewProjection();
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = camera.GetViewMatrix();
        sData->MeshDrawList.clear();
        sData->ColliderDrawList.clear();
    }

    void SceneRenderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        sSceneCBufferData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = glm::inverse(transform);
        sData->MeshDrawList.clear();
        sData->ColliderDrawList.clear();
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
        for (auto& debugMesh : component.ProcessedMeshes)
            sData->ColliderDrawList.push_back({ debugMesh, transform });
    }

    Ref<EnvironmentMap>& SceneRenderer::GetEnvironmentMapSlot()
    {
        return sData->EnvironmentMap;
    }

    bool& SceneRenderer::GetEnvironmentMapActivationBool()
    {
        return sData->EnvironmentMapActivated;
    }

    void SceneRenderer::SetContext(void* editorModule)
    {
        sData->Context = static_cast<EditorModule*>(editorModule);
    }

    void SceneRenderer::SetSceneContext(Scene* sceneContext)
    {
        sData->SceneContext = sceneContext;
    }

    glm::vec2 imageSize = { 200.0f, 200.0f };
    void SceneRenderer::OnImGuiRender()
    {
        ImGui::Begin("Scene Renderer");
        ImGui::Text("Shadow map");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("##a", &imageSize.x);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("##b", &imageSize.y);
        ImGui::PopItemWidth();
        ImGui::Image(static_cast<ImTextureID>(sData->ShadowMapCascades.GetFramebuffers()[0]->GetDepthAttachmentID()), ImVec2(imageSize.x, imageSize.y));
        ImGui::Image(static_cast<ImTextureID>(sData->ShadowMapCascades.GetFramebuffers()[1]->GetDepthAttachmentID()), ImVec2(imageSize.x, imageSize.y));
        ImGui::Image(static_cast<ImTextureID>(sData->ShadowMapCascades.GetFramebuffers()[2]->GetDepthAttachmentID()), ImVec2(imageSize.x, imageSize.y));
        ImGui::End();
    }

    void SceneRenderer::ShadowPass()
    {
        glm::vec3 direction;
        if(sData->SceneContext)
        {
            auto view = sData->SceneContext->mRegistry.view<TransformComponent, DirectionalLightComponent>();
            for (auto entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                direction = glm::normalize(transform.GetTransform()[2]); //Z axis of rotation matrix
            }
        }

        //Calculate the Cascade Ends & ViewProjection matrices
        sData->ShadowMapCascades.CalculateCascadeEnds(sData->Camera.mNearClip, 100);
        sData->ShadowMapCascades.CalculateViewProjection(sData->Camera.GetViewMatrix(), sData->Camera.GetProjection(), direction);

        const float* cascadeEnds = sData->ShadowMapCascades.GetCascadeEnds();
        sData->CascadeEndsCBuffer->SetDynamicData(&cascadeEnds);
        sData->CascadeEndsCBuffer->VSBind();

        RenderCommand::SetCullMode(CullMode::Front);
        //Loop over all the shadow maps and bind and render the whole scene to each of them
        for (Uint j = 0; j < NUM_CASCADES; j++)
        {
            const Ref<Framebuffer>& shadowMapBuffer = sData->ShadowMapCascades.GetFramebuffers()[j];
            shadowMapBuffer->Bind();
            sData->ShadowMapShader->Bind();
            shadowMapBuffer->Clear({ 0.1f, 1.0f, 0.1f, 1.0f });

            //Set the LightSpaceMatrix
            glm::mat4 viewProjection = sData->ShadowMapCascades.GetViewProjections()[j];
            sData->SceneCBuffer->SetDynamicData(&viewProjection);
            sData->SceneCBuffer->VSBind();

            for (DrawCommand& drawCmd : sData->MeshDrawList)
            {
                Ref<Mesh>& mesh = drawCmd.Mesh;
                PipelineSpecification& spec = mesh->GetPipeline()->GetSpecification();
                const Submesh* submeshes = mesh->GetSubmeshes().data();

                //We don't bind the regular PBR shader, so we don't do pipeline->BindSpecificationObjects()
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
        sData->Context->GetFramebuffer()->Bind();
        sData->SceneCBuffer->SetDynamicData(&(*sSceneCBufferData));
        sData->SceneCBuffer->VSBind();

        //Loop over the total number of cascades and set the light ViewProjection
        for (Uint i = 0; i < NUM_CASCADES; i++)
        {
            glm::mat4 viewProjection = sData->ShadowMapCascades.GetViewProjections()[i];
            sData->LightSpaceMatrixCBuffer->SetDynamicData(&viewProjection);
            sData->LightSpaceMatrixCBuffer->VSBind();
        }

        //Bind the shadow maps(which was captured from the ShadowPass()) as texture
        //and draw all the objects in the scene
        sData->ShadowMapCascades.Bind(8);
        for (DrawCommand& drawCmd : sData->MeshDrawList)
            Renderer::DrawMesh(drawCmd.Mesh, drawCmd.Transform);
        sData->ShadowMapCascades.Unbind(8);

        for (DrawCommand& drawCmd : sData->ColliderDrawList)
            Renderer::DrawColliderMesh(drawCmd.Mesh, drawCmd.Transform);
    }

    void SceneRenderer::EndScene()
    {
        //Upload the SceneCBufferData
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

        ShadowPass();
        GeometryPass();

        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);
    }
}
