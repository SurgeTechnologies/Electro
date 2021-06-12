//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer.hpp"
#include "Renderer2D.hpp"
#include "RenderCommand.hpp"
#include "EditorModule.hpp"
#include <imgui_internal.h>
#include "Scene/Components.hpp"
#include <d3d11.h>
#include "Platform/DX11/DX11Internal.hpp"

#define SHADOW_MAP_BINDING_SLOT 8
namespace Electro
{
    ID3D11InputLayout* nullInputLayout;
    Scope<RendererData> Renderer::sData = CreateScope<RendererData>();
    void Renderer::Init()
    {
        // Create All Shaders
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/PBR.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/Skybox.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/EquirectangularToCubemap.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/IrradianceConvolution.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/PreFilterConvolution.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/Renderer2DLine.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/ShadowMap.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/SolidColor.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/Outline.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/Grid.hlsl"));

         /*      CBufferGuide       */
         /*Binding -  Name          */
         /*--------|----------------*/
         /*   0    | Camera         */
         /*   1    | Mesh           */
         /*   2    | Material       */
         /*   3    | Lights         */
         /*   4    | Roughness      */
         /*   5    | Skybox         */
         /*   6    | LightMat       */
         /*   7    | ShadowSettings */
         /*-------------------------*/

        // Create All ConstantBuffers
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 0, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 1, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(LightCBuffer), 3, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 4, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4) * (NUM_CASCADES + 1), 6, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 7, DataUsage::DYNAMIC));

        sData->SceneCBuffer = sData->AllConstantBuffers[0];
        sData->TransformCBuffer = sData->AllConstantBuffers[1];
        sData->LightSpaceMatrixCBuffer = sData->AllConstantBuffers[6];
        sData->ShadowMapShader = GetShader("ShadowMap");
        sData->SolidColorShader = GetShader("SolidColor");
        sData->OutlineShader = GetShader("Outline");
        sData->GridShader = GetShader("Grid");

        // Outline Texture
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.SwapChainTarget = false;
        sData->OutlineTexture = Framebuffer::Create(fbSpec);

        float vert[] =
        {
           -1,  1, 0,
            1,  1, 0,
           -1, -1, 0,

           -1, -1, 0,
            1,  1, 0,
            1, -1, 0
        };

        sData->GridVertexBuffer = VertexBuffer::Create(vert, sizeof(vert));
        sData->GridPipeline = Pipeline::Create();
        sData->GridPipeline->GenerateInputLayout(sData->GridShader);

        sData->Shadows.Init();
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

    void Renderer::SubmitOutlineMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        sData->OutlineDrawList.push_back(DrawCommand(mesh, transform));
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
        sData->Shadows.CalculateMatricesAndSetShadowCBufferData(sData->ViewMatrix, sData->ProjectionMatrix, glm::normalize(direction));

        // Loop over all the shadow maps and bind and render the whole scene to each of them
        for (Uint j = 0; j < NUM_CASCADES; j++)
        {
            const Ref<Framebuffer>& shadowMapBuffer = sData->Shadows.GetFramebuffers()[j];
            shadowMapBuffer->Bind();
            sData->ShadowMapShader->Bind();
            shadowMapBuffer->Clear();

            // Set the LightSpaceMatrix
            sData->SceneCBuffer->SetDynamicData((void*)(&sData->Shadows.GetViewProjections()[j]));
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
                    sData->TransformCBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                    sData->TransformCBuffer->VSBind();
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }
        }
    }

    void Renderer::DebugPass()
    {
        sData->SceneCBuffer->VSBind();
        sData->SceneCBuffer->SetDynamicData(&sData->ViewProjectionMatrix);
        Renderer2D::BeginScene(sData->ViewProjectionMatrix);
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
        // Outline
        if (!sData->OutlineDrawList.empty())
        {
            sData->OutlineTexture->Bind();
            sData->OutlineTexture->Clear({ 0.1f, 0.1f, 0.1f, 1.0f });
            RenderCommand::DisableDepth();
            for (const DrawCommand& drawCmd : sData->OutlineDrawList)
            {
                const Ref<Mesh>& mesh = drawCmd.Mesh;
                const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                mesh->GetIndexBuffer()->Bind();
                pipeline->Bind();

                const Submesh* submeshes = mesh->GetSubmeshes().data();
                for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
                {
                    const Submesh& submesh = submeshes[i];
                    sData->TransformCBuffer->VSBind();
                    sData->TransformCBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                    sData->SolidColorShader->Bind();
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }
            }
            sData->ActiveRenderBuffer->Bind();
            sData->OutlineTexture->BindColorBufferAsTexture(0, 0);
            RenderFullscreenQuad();
            sData->OutlineTexture->UnbindColorBufferAsTexture(0);
            RenderCommand::EnableDepth();
        }
        if (sData->ShowGrid)
        {
            // TODO
            sData->GridPipeline->Bind();
            sData->GridVertexBuffer->Bind(sData->GridPipeline->GetStride());
            sData->GridShader->Bind();
            RenderCommand::Draw(6);
        }
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
            lightMatData[i] = sData->Shadows.GetViewProjections()[i];

        // Set the the view matrix at the last index
        lightMatData[NUM_CASCADES] = sData->ViewMatrix;

        // Set the LightMatrix Data to the Vertex shader
        sData->LightSpaceMatrixCBuffer->SetDynamicData(lightMatData);
        sData->LightSpaceMatrixCBuffer->VSBind();

        // Bind the shadow maps(which was captured from the ShadowPass()) as texture and draw all the objects in the scene
        // NOTE: Here starting slot is SHADOW_MAP_BINDING_SLOT = 8, so the shadow maps gets bound as 8, 9, 10, ..., n
        sData->Shadows.Bind(SHADOW_MAP_BINDING_SLOT);
        for (const DrawCommand& drawCmd : sData->MeshDrawList)
        {
            const Ref<Mesh>& mesh = drawCmd.Mesh;
            const Ref<Pipeline>& pipeline = mesh->GetPipeline();
            mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
            mesh->GetIndexBuffer()->Bind();
            pipeline->Bind();

            const Vector<Ref<Material>>& materials = mesh->GetMaterials();
            const Submesh* submeshes = mesh->GetSubmeshes().data();

            // Render all the submeshes
            for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
            {
                const Submesh& submesh = submeshes[i];
                materials[submesh.MaterialIndex]->Bind();

                sData->TransformCBuffer->VSBind();
                sData->TransformCBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            }
        }
        sData->Shadows.Unbind(SHADOW_MAP_BINDING_SLOT);
    }

    void Renderer::EndScene()
    {
        ShadowPass();
        GeometryPass();

        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);

        if (!sData->SceneContext->mIsRuntimeScene)
            DebugPass(); // We only Render Debug symbols in edit mode

        ClearDrawList();
    }

    bool Renderer::IsDrawListEmpty() { return (sData->MeshDrawList.empty() && sData->OutlineDrawList.empty()); }
    void Renderer::ClearDrawList()
    {
        sData->MeshDrawList.clear();
        sData->OutlineDrawList.clear();
    }

    void Renderer::RenderFullscreenQuad()
    {
        sData->OutlineShader->Bind();
        RenderCommand::Draw(3);
    }

    const Ref<Shader> Renderer::GetShader(const String& nameWithoutExtension)
    {
        for (const Ref<Shader>& shader : sData->AllShaders)
        {
            if (FileSystem::GetNameWithoutExtension(shader->GetPath()) == nameWithoutExtension)
                return shader;
        }
        return Ref<Shader>(nullptr);
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::ResizeBackbuffer(0, 0, width, height);
    }
}
