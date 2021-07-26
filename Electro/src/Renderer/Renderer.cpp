//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer.hpp"
#include "Renderer2D.hpp"
#include "RenderCommand.hpp"
#include "EditorModule.hpp"
#include "Scene/Components.hpp"
#include "Math/Math.hpp"

#define SHADOW_MAP_BINDING_SLOT 8
namespace Electro
{
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
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/GaussianBlur.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/ThresholdDownsampleShader.hlsl"));
        sData->AllShaders.emplace_back(Shader::Create("Electro/assets/shaders/HLSL/QuadComposite.hlsl"));

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
         /*   8    | InverseCamera  */
         /*   9    | Color          */
         /*  10    | BlurParams     */
         /*  11    | BloomThreshold */
         /*  12    | BloomExposure  */
         /*-------------------------*/

        // Create All ConstantBuffers (TODO: Come up with a proper way of managing all Constant Buffers)
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 0, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 1, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(LightCBuffer), 3, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 4, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(Ref<ConstantBuffer>(nullptr)); // Used via Material
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4) * (NUM_CASCADES + 1), 6, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 7, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::mat4), 8, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 9, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(BlurParams), 10, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 11, DataUsage::DYNAMIC));
        sData->AllConstantBuffers.emplace_back(ConstantBuffer::Create(sizeof(glm::vec4), 12, DataUsage::DYNAMIC));

        sData->SceneCBuffer = sData->AllConstantBuffers[0];
        sData->TransformCBuffer = sData->AllConstantBuffers[1];
        sData->LightConstantBuffer = sData->AllConstantBuffers[3];
        sData->LightSpaceMatrixCBuffer = sData->AllConstantBuffers[6];
        sData->InverseViewProjectionCBuffer = sData->AllConstantBuffers[8];
        sData->SolidColorCBuffer = sData->AllConstantBuffers[9];
        sData->ExposureCBuffer = sData->AllConstantBuffers[12];

        sData->ShadowMapShader = GetShader("ShadowMap");
        sData->SolidColorShader = GetShader("SolidColor");
        sData->OutlineShader = GetShader("Outline");
        sData->GridShader = GetShader("Grid");
        sData->QuadCompositeShader = GetShader("QuadComposite");

        Uint width = 1280;
        Uint height = 720;

        {   // Outline Texture
            RenderbufferSpecification fbSpec;
            fbSpec.Attachments = { RenderBufferTextureFormat::RGBA32F };
            fbSpec.Width = width;
            fbSpec.Height = height;
            fbSpec.SwapChainTarget = false;
            sData->OutlineRenderBuffer = Renderbuffer::Create(fbSpec);
        }
        {
            RenderbufferSpecification fbSpec;
            fbSpec.Attachments = { RenderBufferTextureFormat::RGBA32F, RenderBufferTextureFormat::DEPTH };
            fbSpec.Width = width;
            fbSpec.Height = height;
            fbSpec.SwapChainTarget = false;
            sData->GeometryBuffer = Renderbuffer::Create(fbSpec);
            sData->FinalSceneBuffer = Renderbuffer::Create(fbSpec);
        }

        sData->Shadows.Init();
        sData->PostProcessPipeline.Init(sData->GeometryBuffer);
    }

    void Renderer::Shutdown()
    {
        sData->PostProcessPipeline.Shutdown();
        sData.release();
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to Renderer::BeginScene() without matching Renderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetViewProjection();
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = camera.GetViewMatrix();
        sData->CameraPosition = camera.GetPosition();
        sData->TotalDrawCalls = 0;
    }

    void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        E_ASSERT(IsDrawListEmpty(), "Call to Renderer::BeginScene() without matching Renderer::EndScene(...)!");
        sData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
        sData->ProjectionMatrix = camera.GetProjection();
        sData->ViewMatrix = glm::inverse(transform);

        glm::vec3 translation, rotation, scale;
        Math::DecomposeTransform(transform, translation, rotation, scale);
        sData->CameraPosition = translation;
        sData->TotalDrawCalls = 0;
    }

    void Renderer::SubmitMesh(const MeshComponent& mesh, const glm::mat4& transform)
    {
        sData->MeshDrawList.emplace_back(mesh, transform);
    }

    void Renderer::SubmitOutlineMesh(const MeshComponent& mesh, const glm::mat4& transform)
    {
        sData->OutlineDrawList.emplace_back(mesh, transform);
    }

    void Renderer::SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform, bool show)
    {
        for (const Ref<Mesh>& debugMesh : component.ProcessedMeshes)
            sData->ColliderDrawList.emplace_back(debugMesh, transform, show);
    }

    void Renderer::SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& transform, bool show)
    {
        sData->ColliderDrawList.emplace_back(component.DebugMesh, transform, show);
    }

    void Renderer::SubmitPointLight(const PointLight& pointLight)
    {
        sData->AllPointLights.emplace_back(pointLight);
    }

    void Renderer::SubmitDirectionalLight(const DirectionalLight& directionalLight)
    {
        sData->AllDirectionalLights.emplace_back(directionalLight);
    }

    void Renderer::ShadowPass()
    {
        glm::vec3 direction;
        if (sData->SceneContext)
        {
            auto view = sData->SceneContext->GetRegistry().view<TransformComponent, DirectionalLightComponent>();
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
            const Ref<Renderbuffer>& shadowMapBuffer = sData->Shadows.GetFramebuffers()[j];
            shadowMapBuffer->Bind();
            sData->ShadowMapShader->Bind();
            shadowMapBuffer->Clear();

            // Set the LightSpaceMatrix
            sData->SceneCBuffer->VSBind();
            sData->SceneCBuffer->SetDynamicData((void*)(&sData->Shadows.GetViewProjections()[j]));

            for (const DrawCommand& drawCmd : sData->MeshDrawList)
            {
                if (drawCmd.GetMeshComponent().CastShadows)
                {
                    const Ref<Mesh>& mesh = drawCmd.GetMesh();
                    const Submesh* submeshes = mesh->GetSubmeshes().data();

                    const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                    mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                    mesh->GetIndexBuffer()->Bind();
                    pipeline->Bind();

                    for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
                    {
                        const Submesh& submesh = submeshes[i];
                        sData->TransformCBuffer->VSBind();
                        sData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                        RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                        sData->TotalDrawCalls++;
                    }
                    pipeline->Unbind();
                }
            }
        }
    }

    void Renderer::DebugPass()
    {
        sData->GeometryBuffer->Bind();
        sData->SceneCBuffer->VSBind();
        sData->SceneCBuffer->SetDynamicData(&sData->ViewProjectionMatrix);
        Renderer2D::BeginScene(sData->ViewProjectionMatrix);
        if (sData->ShowCameraFrustum)
        {
            glm::mat4 cameraView;
            glm::mat4 cameraProjection;
            {
                auto view = sData->SceneContext->GetRegistry().view<TransformComponent, CameraComponent>();
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
                const Vector<Submesh>& submeshes = drawCmd.GetMesh()->GetSubmeshes();
                for (Uint i = 0; i < submeshes.size(); i++)
                {
                    const Submesh& submesh = submeshes[i];
                    Renderer2D::SubmitAABB(submesh.BoundingBox, drawCmd.GetTransform() * submesh.Transform);
                }
            }
        }
        Renderer2D::EndScene();

        // Outline
        if (!sData->OutlineDrawList.empty())
        {
            sData->OutlineRenderBuffer->Bind();
            sData->OutlineRenderBuffer->Clear({ 0.1f, 0.1f, 0.1f, 1.0f });
            RenderCommand::DisableDepth();
            for (const DrawCommand& drawCmd : sData->OutlineDrawList)
            {
                const Ref<Mesh>& mesh = drawCmd.GetMesh();
                const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                mesh->GetIndexBuffer()->Bind();
                pipeline->Bind();

                const Submesh* submeshes = mesh->GetSubmeshes().data();
                for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
                {
                    const Submesh& submesh = submeshes[i];
                    glm::vec4 color = { 1.0f, 0.5f, 0.0f, 1.0f };

                    sData->TransformCBuffer->VSBind();
                    sData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                    sData->SolidColorShader->Bind();
                    sData->SolidColorCBuffer->PSBind();
                    sData->SolidColorCBuffer->SetDynamicData(&color);
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                    sData->TotalDrawCalls++;
                }

                pipeline->Unbind();
            }
            sData->OutlineRenderBuffer->Unbind();

            sData->GeometryBuffer->Bind();
            sData->OutlineShader->Bind();
            sData->OutlineRenderBuffer->BindColorBuffer(0, 0, ShaderDomain::PIXEL);
            RenderFullscreenQuad();
            sData->OutlineRenderBuffer->UnbindBuffer(0, ShaderDomain::PIXEL);
            RenderCommand::EnableDepth();
        }
        if (sData->ShowGrid)
        {
            sData->GridShader->Bind();
            RenderCommand::Draw(3); // Render Full screen Quad, but with depth enabled
        }
        // Collider
        {
            for (const ColliderDrawCommand& drawCmd : sData->ColliderDrawList)
            {
                if (drawCmd.Show)
                {
                    const Ref<Mesh>& mesh = drawCmd.Mesh;
                    const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                    glm::vec4 color = { 0.1f, 0.9f, 0.1f, 1.0f };

                    pipeline->Bind();
                    mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                    mesh->GetIndexBuffer()->Bind();
                    sData->SolidColorShader->Bind();
                    sData->SolidColorCBuffer->PSBind();
                    sData->SolidColorCBuffer->SetDynamicData(&color);

                    RenderCommand::BeginWireframe();
                    for (const Submesh& submesh : mesh->GetSubmeshes())
                    {
                        sData->TransformCBuffer->VSBind();
                        sData->TransformCBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                        RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                        sData->TotalDrawCalls++;
                    }
                    RenderCommand::EndWireframe();
                    pipeline->Unbind();
                }
            }
        }
        sData->GeometryBuffer->Unbind();
    }

    void Renderer::GeometryPass()
    {
        sData->GeometryBuffer->Bind();
        sData->FinalSceneBuffer->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
        sData->GeometryBuffer->Clear({ 0.05f, 0.05f, 0.05f, 1.0f });

        sData->InverseViewProjectionCBuffer->VSBind();
        sData->InverseViewProjectionCBuffer->SetDynamicData(&glm::inverse(sData->ViewProjectionMatrix));

        sData->SceneCBuffer->VSBind();
        sData->SceneCBuffer->SetDynamicData(&sData->ViewProjectionMatrix);

        // Loop over the total number of cascades and set the light ViewProjection
        glm::mat4 lightMatData[NUM_CASCADES + 1];
        for (Uint i = 0; i < NUM_CASCADES; i++)
            lightMatData[i] = sData->Shadows.GetViewProjections()[i];

        // Set the the view matrix at the last index
        lightMatData[NUM_CASCADES] = sData->ViewMatrix;

        // Set the LightMatrix Data to the Vertex shader
        sData->LightSpaceMatrixCBuffer->VSBind();
        sData->LightSpaceMatrixCBuffer->SetDynamicData(lightMatData);

        CalculateAndRenderLights(sData->CameraPosition);

        // Bind the shadow maps(which was captured from the ShadowPass()) as texture and draw all the objects in the scene
        //! NOTE: Here starting slot is SHADOW_MAP_BINDING_SLOT = 8, so the shadow maps gets bound as 8, 9, 10, ..., n
        if (sData->ShadowsEnabled)
            sData->Shadows.Bind(SHADOW_MAP_BINDING_SLOT);

        for (const DrawCommand& drawCmd : sData->MeshDrawList)
        {
            const Ref<Mesh>& mesh = drawCmd.GetMesh();
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
                sData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                sData->TotalDrawCalls++;

                materials[submesh.MaterialIndex]->Unbind();
            }

            pipeline->Unbind();
        }

        sData->Shadows.Unbind(SHADOW_MAP_BINDING_SLOT);
        ClearLights();
        sData->GeometryBuffer->Unbind();
    }

    void Renderer::PostProcessing()
    {
        sData->PostProcessPipeline.ProcessAll();
    }

    void Renderer::CompositePass()
    {
        glm::vec4 exposureParams = { sData->Exposure, 0.0f, 0.0f, 0.0f };
        const Ref<Renderbuffer>& bloomResult = sData->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY)->GetOutputRenderBuffer();

        // We are now rendering to the Final Scene RendererBuffer
        if (sData->RenderToSwapChain)
            RenderCommand::BindBackbuffer();
        else
            sData->FinalSceneBuffer->Bind();

        sData->QuadCompositeShader->Bind();

        // Bind the Blurred texture and the Geometry Buffer
        sData->GeometryBuffer->BindColorBuffer(0, 0, ShaderDomain::PIXEL);
        bloomResult->BindColorBuffer(0, 1, ShaderDomain::PIXEL);

        sData->ExposureCBuffer->SetDynamicData(&exposureParams);
        sData->ExposureCBuffer->PSBind();

        RenderFullscreenQuad();

        // Unbind the Blurred texture and the Geometry Buffer
        bloomResult->UnbindBuffer(1, ShaderDomain::PIXEL);
        sData->GeometryBuffer->UnbindBuffer(0, ShaderDomain::PIXEL);

        if (sData->RenderToSwapChain)
            RenderCommand::GetBackBuffer()->Unbind();
        else
            sData->FinalSceneBuffer->Unbind();
    }

    void Renderer::EndScene()
    {
        if (sData->ShadowsEnabled)
            ShadowPass();

        GeometryPass();

        sData->GeometryBuffer->Bind();
        if (sData->EnvironmentMap && sData->EnvironmentMapActivated)
            sData->EnvironmentMap->Render(sData->ProjectionMatrix, sData->ViewMatrix);
        sData->GeometryBuffer->Unbind();

        if (!sData->SceneContext->IsRuntimeScene())
            DebugPass(); // We only Render Debug symbols in edit mode

        PostProcessing();
        CompositePass();
        ClearDrawList();
    }

    bool Renderer::IsDrawListEmpty() { return sData->MeshDrawList.empty(); }
    void Renderer::ClearDrawList()
    {
        sData->MeshDrawList.clear();
        sData->OutlineDrawList.clear();
        sData->ColliderDrawList.clear();
    }

    void Renderer::RenderFullscreenQuad()
    {
        RenderCommand::DisableDepth();
        RenderCommand::Draw(3);
        RenderCommand::EnableDepth();
    }

    void Renderer::ClearLights()
    {
        sData->AllDirectionalLights.clear();
        sData->AllPointLights.clear();
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

    void Renderer::CalculateAndRenderLights(const glm::vec3& cameraPos)
    {
        sData->LightCBufferData.CameraPosition = cameraPos;
        sData->LightCBufferData.PointLightCount = static_cast<Uint>(sData->AllPointLights.size());
        sData->LightCBufferData.DirectionalLightCount = static_cast<Uint>(sData->AllDirectionalLights.size());

        for (int i = 0; i < sData->AllPointLights.size(); i++)
        {
            auto& light = sData->AllPointLights[i];
            sData->LightCBufferData.PointLights[i].Position = light.Position;
            sData->LightCBufferData.PointLights[i].Intensity = light.Intensity;
            sData->LightCBufferData.PointLights[i].Color = light.Color;
            sData->LightCBufferData.PointLights[i].Radius = light.Radius;
        }

        for (int i = 0; i < sData->AllDirectionalLights.size(); i++)
        {
            auto& light = sData->AllDirectionalLights[i];
            sData->LightCBufferData.DirectionalLights[i].Direction = light.Direction;
            sData->LightCBufferData.DirectionalLights[i].Intensity = light.Intensity;
            sData->LightCBufferData.DirectionalLights[i].Color = light.Color;
        }

        sData->LightConstantBuffer->PSBind();
        sData->LightConstantBuffer->SetDynamicData(&sData->LightCBufferData);
    }

    void Renderer::RenderToSwapchain()
    {
        sData->RenderToSwapChain = true;
    }
}
