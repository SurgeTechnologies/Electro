//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer.hpp"
#include "Renderer2D.hpp"
#include "RenderCommand.hpp"
#include "EditorModule.hpp"
#include <imgui_internal.h>

#define SHADOW_MAP_BINDING_SLOT 8
namespace Electro
{
    static Scope<RendererData> sData = CreateScope<RendererData>();
    void Renderer::Init()
    {
        // Creates and submits all the shader to AssetManager
        Factory::CreateShader("Electro/assets/shaders/HLSL/PBR.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/Skybox.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/EquirectangularToCubemap.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/IrradianceConvolution.hlsl");
        Factory::CreateShader("Electro/assets/shaders/HLSL/PreFilterConvolution.hlsl");
        sData->ShadowMapShader = Factory::CreateShader("Electro/assets/shaders/HLSL/ShadowMap.hlsl");
        sData->ColliderShader = Factory::CreateShader("Electro/assets/shaders/HLSL/Collider.hlsl");

        sData->ShadowMapCascades.Init();

        // Cascade matrices size is NUM_CASCADES and '+ 1' is for view matrix
        sData->LightSpaceMatrixCBuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4) * (NUM_CASCADES + 1), 6, DataUsage::DYNAMIC);
        sData->SceneCBuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);

        // Grid
        int count = 11;
        float length = 10;
        for (float j = 0; j < count; ++j)
        {
            sData->GridPositions.push_back({ glm::vec3(j, 0, -length), glm::vec3(j, 0, length) });
            sData->GridPositions.push_back({ glm::vec3(-length, 0, j), glm::vec3(length, 0, j) });
        }
        for (float j = 0; j > -count; --j)
        {
            sData->GridPositions.push_back({ glm::vec3(j, 0, -length), glm::vec3(j, 0, length) });
            sData->GridPositions.push_back({ glm::vec3(-length, 0, j), glm::vec3(length, 0, j) });
        }
    }

    void Renderer::Shutdown()
    {
        sData.release();
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to Renderer::BeginScene() without matching Renderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetViewProjection();
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = camera.GetViewMatrix();
    }

    void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to Renderer::BeginScene() without matching Renderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = glm::inverse(transform);
    }

    void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        sData->MeshDrawList.push_back(DrawCommand(mesh, transform));
    }

    void Renderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform)
    {
        sData->ColliderDrawList.push_back(DrawCommand(component.DebugMesh, transform));
    }

    void Renderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform)
    {
        sData->ColliderDrawList.push_back(DrawCommand(component.DebugMesh, transform));
    }

    void Renderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform)
    {
        for (const Ref<Mesh>& debugMesh : component.ProcessedMeshes)
            sData->ColliderDrawList.push_back(DrawCommand(debugMesh, transform));
    }

    void Renderer::ShadowPass()
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
        sData->ShadowMapCascades.CalculateMatricesAndSetShadowCBufferData(sData->ViewMatrix, sData->ProjectionMatrix, glm::normalize(direction));

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
                const Submesh* submeshes = mesh->GetSubmeshes().data();

                const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                mesh->GetIndexBuffer()->Bind();
                pipeline->Bind();

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

    void Renderer::DebugPass()
    {
        Renderer2D::BeginScene(sData->ProjectionMatrix * sData->ViewMatrix);
        if (sData->ShowCameraFrustum)
        {
            glm::mat4 cameraView;
            glm::mat4 cameraProjection;
            {
                auto view = sData->SceneContext->mRegistry.view<TransformComponent, CameraComponent>();
                for (auto entity : view)
                {
                    auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                    if (camera.Primary)
                    {
                        cameraView = transform.GetTransform();
                        cameraProjection = camera.Camera.GetProjection(); break;
                    }
                }
            }
            const glm::mat4 inv = cameraView * glm::inverse(cameraProjection);
            glm::vec4 frustumCorners[8] =
            {
                // Near face
                {  1.0f,  1.0f, -1.0f, 1.0f },
                { -1.0f,  1.0f, -1.0f, 1.0f },
                {  1.0f, -1.0f, -1.0f, 1.0f },
                { -1.0f, -1.0f, -1.0f, 1.0f },

                // Far face
                {  1.0f,  1.0f, 1.0f, 1.0f },
                { -1.0f,  1.0f, 1.0f, 1.0f },
                {  1.0f, -1.0f, 1.0f, 1.0f },
                { -1.0f, -1.0f, 1.0f, 1.0f },
            };

            glm::vec3 v[8];
            for (Uint i = 0; i < 8; i++)
            {
                const glm::vec4 ff = inv * frustumCorners[i];
                v[i].x = ff.x / ff.w;
                v[i].y = ff.y / ff.w;
                v[i].z = ff.z / ff.w;
            }

            Renderer2D::SubmitLine(v[0], v[1]);
            Renderer2D::SubmitLine(v[0], v[2]);
            Renderer2D::SubmitLine(v[3], v[1]);
            Renderer2D::SubmitLine(v[3], v[2]);

            Renderer2D::SubmitLine(v[4], v[5]);
            Renderer2D::SubmitLine(v[4], v[6]);
            Renderer2D::SubmitLine(v[7], v[5]);
            Renderer2D::SubmitLine(v[7], v[6]);

            Renderer2D::SubmitLine(v[0], v[4]);
            Renderer2D::SubmitLine(v[1], v[5]);
            Renderer2D::SubmitLine(v[3], v[7]);
            Renderer2D::SubmitLine(v[2], v[6]);
        }
        if (sData->ShowGrid)
        {
            for (const Line& pos : sData->GridPositions)
                Renderer2D::SubmitLine(pos.P1, pos.P2, { 0.2f, 0.7f, 0.2f, 1.0f });
        }
        if (sData->ShowAABB)
        {
            for (const DrawCommand& drawCmd : sData->MeshDrawList)
            {
                const Vector<Submesh>& submeshes = drawCmd.Mesh->GetSubmeshes();
                for (Uint i = 0; i < submeshes.size(); i++)
                {
                    const Submesh& submesh = submeshes[i];
                    Renderer2D::SubmitAABB(submesh.BoundingBox, drawCmd.Transform * submesh.Transform);
                }
            }
        }
        Renderer2D::EndScene();
    }

    void Renderer::GeometryPass()
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

        // Set the the view matrix at the last index
        lightMatData[NUM_CASCADES] = sData->ViewMatrix;

        // Set the LightMatrix Data to the Vertex shader
        sData->LightSpaceMatrixCBuffer->SetDynamicData(lightMatData);
        sData->LightSpaceMatrixCBuffer->VSBind();

        // Bind the shadow maps(which was captured from the ShadowPass()) as texture and draw all the objects in the scene
        // NOTE: Here starting slot is SHADOW_MAP_BINDING_SLOT = 8, so the shadow maps gets bound as 8, 9, 10, ..., n
        sData->ShadowMapCascades.Bind(SHADOW_MAP_BINDING_SLOT);
        for (const DrawCommand& drawCmd : sData->MeshDrawList)
        {
            const Ref<Mesh>& mesh = drawCmd.Mesh;
            const Ref<Pipeline>& pipeline = mesh->GetPipeline();
            mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
            mesh->GetIndexBuffer()->Bind();
            pipeline->Bind();

            const Vector<Ref<Material>>& materials = mesh->GetMaterials();
            const Submesh* submeshes = mesh->GetSubmeshes().data();

            for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
            {
                const Submesh& submesh = submeshes[i];
                materials[submesh.MaterialIndex]->Bind();

                submesh.CBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                submesh.CBuffer->VSBind();
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            }
        }
        sData->ShadowMapCascades.Unbind(SHADOW_MAP_BINDING_SLOT);

        // Render the colliders
        for (const DrawCommand& drawCmd : sData->ColliderDrawList)
        {
            if (drawCmd.Mesh)
            {
                const Ref<Pipeline>& pipeline = drawCmd.Mesh->GetPipeline();

                sData->ColliderShader->Bind();
                drawCmd.Mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                drawCmd.Mesh->GetIndexBuffer()->Bind();
                pipeline->Bind();

                RenderCommand::BeginWireframe();
                for (const Submesh& submesh : drawCmd.Mesh->GetSubmeshes())
                {
                    submesh.CBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                    submesh.CBuffer->VSBind();
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
                RenderCommand::EndWireframe();
            }
        }
        sData->ActiveRenderBuffer->Unbind();
    }

    void Renderer::EndScene()
    {
        ShadowPass();
        GeometryPass();
        DebugPass();

        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);

        ClearDrawList();
    }

    bool Renderer::IsDrawListEmpty() { return (sData->MeshDrawList.empty() && sData->ColliderDrawList.empty()); }
    void Renderer::ClearDrawList()
    {
        sData->MeshDrawList.clear();
        sData->ColliderDrawList.clear();
    }

    void Renderer::SetSceneContext(Scene* sceneContext)
    {
        sData->SceneContext = sceneContext;
    }

    void Renderer::SetActiveRenderBuffer(Ref<Framebuffer>& renderBuffer)
    {
        sData->ActiveRenderBuffer = renderBuffer;
    }

    const Scope<RendererData>& Renderer::GetData()
    {
        return sData;
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::ResizeBackbuffer(0, 0, width, height);
    }
}
