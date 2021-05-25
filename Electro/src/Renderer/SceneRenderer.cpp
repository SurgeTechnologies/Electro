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
#include "RenderCommand.hpp"
#include "EditorModule.hpp"
#include "RendererDebug.hpp"
#include "Platform/DX11/DX11Internal.hpp"
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
        Ref<ConstantBuffer> SceneCbuffer;
        Ref<ConstantBuffer> LightSpaceMatrixCbuffer;
        size_t DrawCalls = 0;
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> ColliderDrawList;

        //Environment Map
        Ref<EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        //Shadows
        Ref<Shader> ShadowMapShader;
        Ref<Framebuffer> ShadowFramebuffer;
        glm::mat4 LightSpaceMatrix;
        BoundingBox SceneAABB;
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
        sSceneData->ShadowMapShader = Factory::CreateShader("Electro/assets/shaders/HLSL/ShadowMap.hlsl");

        {
            FramebufferSpecification fbSpec;
            fbSpec.Attachments = { FramebufferTextureFormat::R32_TYPELESS };
            fbSpec.Width = 1024;
            fbSpec.Height = 1024;
            fbSpec.SwapChainTarget = false;
            sSceneData->ShadowFramebuffer = Factory::CreateFramebuffer(fbSpec);
        }

        sSceneData->SceneCbuffer = Factory::CreateConstantBuffer(sizeof(SceneCBufferData), 0, DataUsage::DYNAMIC);
        sSceneData->LightSpaceMatrixCbuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 6, DataUsage::DYNAMIC);
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

    void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
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

    void SceneRenderer::SetContext(void* editorModule)
    {
        sSceneData->Context = static_cast<EditorModule*>(editorModule);
    }

    void SceneRenderer::SetSceneContext(Scene* sceneContext)
    {
        sSceneData->SceneContext = sceneContext;
    }

    float nearPlane = 0.0001f;
    float farPlane = 7.5f;
    void SceneRenderer::OnImGuiRender()
    {
        ImGui::Begin("Shadows");
        ImGui::DragFloat("Near plane", &nearPlane, 1.0f, 0.0f, 0.0f, "%.6f");
        ImGui::DragFloat("Far plane", &farPlane);
        ImGui::End();
    }

    Vector<glm::vec3> ExpandAABB(BoundingBox bb)
    {
        Vector<glm::vec3> b(8);
        b[0] = { bb.Min.x, bb.Min.y, bb.Min.z };
        b[1] = { bb.Max.x, bb.Min.y, bb.Min.z };
        b[2] = { bb.Max.x, bb.Max.y, bb.Min.z };
        b[3] = { bb.Min.x, bb.Max.y, bb.Min.z };
        b[4] = { bb.Min.x, bb.Min.y, bb.Max.z };
        b[5] = { bb.Max.x, bb.Min.y, bb.Max.z };
        b[6] = { bb.Max.x, bb.Max.y, bb.Max.z };
        b[7] = { bb.Min.x, bb.Max.y, bb.Max.z };
        return b;
    }

    void SceneRenderer::ShadowPass()
    {
        glm::mat4 lightView;
        if(sSceneData->SceneContext)
        {
            {
                auto view = sSceneData->SceneContext->mRegistry.view<TransformComponent, DirectionalLightComponent>();
                for (auto entity : view)
                {
                    auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                    glm::vec3 direction = glm::normalize(transform.GetTransform()[2]); //Z axis of rotation matrix
                    lightView = glm::lookAt(transform.Translation, direction, glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
        }

#if 0
        BoundingBox lightSpaceAABB = { glm::vec3(1.0f), glm::vec3(1.0f) };
        Vector<glm::vec3> expanded = ExpandAABB(sSceneData->SceneAABB);
        for (glm::vec3& point : expanded)
        {
            const glm::vec4 pointinview = lightView * glm::vec4(point, 1.0f);

            lightSpaceAABB.Min.x = pointinview.x < lightSpaceAABB.Min.x ? pointinview.x : lightSpaceAABB.Min.x;
            lightSpaceAABB.Min.y = pointinview.y < lightSpaceAABB.Min.y ? pointinview.y : lightSpaceAABB.Min.y;
            lightSpaceAABB.Min.z = pointinview.z < lightSpaceAABB.Min.z ? pointinview.z : lightSpaceAABB.Min.z;

            lightSpaceAABB.Max.x = pointinview.x > lightSpaceAABB.Max.x ? pointinview.x : lightSpaceAABB.Max.x;
            lightSpaceAABB.Max.y = pointinview.y > lightSpaceAABB.Max.y ? pointinview.y : lightSpaceAABB.Max.y;
            lightSpaceAABB.Max.z = pointinview.z > lightSpaceAABB.Max.z ? pointinview.z : lightSpaceAABB.Max.z;
        }
                //Light Projection Matrix 
        const glm::mat4 lightProjection = glm::ortho(lightSpaceAABB.Min.x, lightSpaceAABB.Max.x, 
                                                     lightSpaceAABB.Min.y, lightSpaceAABB.Max.y, 
                                                     lightSpaceAABB.Min.z, lightSpaceAABB.Max.z);
#endif

        const glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
        sSceneData->LightSpaceMatrix = lightProjection * lightView;

        sSceneData->ShadowFramebuffer->Bind();
        sSceneData->ShadowFramebuffer->Clear({ 0.1f, 1.0f, 0.1f, 1.0f });
        sSceneData->ShadowMapShader->Bind();
        sSceneData->SceneCbuffer->SetDynamicData(&sSceneData->LightSpaceMatrix);
        sSceneData->SceneCbuffer->VSBind();

        RenderCommand::SetCullMode(CullMode::Front);
        for (DrawCommand& drawCmd : sSceneData->MeshDrawList)
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
                CalculateSceneAABB(submesh.LocalTransform, submesh);
                submesh.CBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                submesh.CBuffer->VSBind();
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            }
        }
        RenderCommand::SetCullMode(CullMode::None); //TODO: Set to CullMode::Back?
        sSceneData->ShadowFramebuffer->Unbind();
    }

    void SceneRenderer::CalculateSceneAABB(const glm::mat4& transform, const Submesh& mesh)
    {
        glm::vec4 min = transform * glm::vec4(mesh.BoundingBox.Min, 1.0f);
        sSceneData->SceneAABB.Min.x = min.x < sSceneData->SceneAABB.Min.x ? min.x : sSceneData->SceneAABB.Min.x;
        sSceneData->SceneAABB.Min.y = min.y < sSceneData->SceneAABB.Min.y ? min.y : sSceneData->SceneAABB.Min.y;
        sSceneData->SceneAABB.Min.z = min.z < sSceneData->SceneAABB.Min.z ? min.z : sSceneData->SceneAABB.Min.z;
        glm::vec4 max = transform * glm::vec4(mesh.BoundingBox.Max, 1.0f);
        sSceneData->SceneAABB.Max.x = max.x > sSceneData->SceneAABB.Max.x ? max.x : sSceneData->SceneAABB.Max.x;
        sSceneData->SceneAABB.Max.y = max.y > sSceneData->SceneAABB.Max.y ? max.y : sSceneData->SceneAABB.Max.y;
        sSceneData->SceneAABB.Max.z = max.z > sSceneData->SceneAABB.Max.z ? max.z : sSceneData->SceneAABB.Max.z;
    }

    void SceneRenderer::GeometryPass()
    {
        sSceneData->Context->GetFramebuffer()->Bind();
        sSceneData->SceneCbuffer->SetDynamicData(&(*sSceneCBufferData));
        sSceneData->SceneCbuffer->VSBind();

        sSceneData->LightSpaceMatrixCbuffer->SetDynamicData(&(sSceneData->LightSpaceMatrix));
        sSceneData->LightSpaceMatrixCbuffer->VSBind();

        ID3D11ShaderResourceView* null = nullptr;
        ID3D11ShaderResourceView* dsrv = static_cast<ID3D11ShaderResourceView*>(sSceneData->ShadowFramebuffer->GetDepthAttachmentID());
        DX11Internal::GetDeviceContext()->PSSetShaderResources(8, 1, &dsrv);

        for (DrawCommand& drawCmd : sSceneData->MeshDrawList)
            Renderer::DrawMesh(drawCmd.Mesh, drawCmd.Transform);

        for (DrawCommand& drawCmd : sSceneData->ColliderDrawList)
            Renderer::DrawColliderMesh(drawCmd.Mesh, drawCmd.Transform);

        DX11Internal::GetDeviceContext()->PSSetShaderResources(8, 1, &null);

        sSceneData->Context->GetFramebuffer()->Unbind();
    }

    void SceneRenderer::EndScene()
    {
        //Upload the SceneCBufferData
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

        ShadowPass();
        GeometryPass();

        if (sSceneData->EnvironmentMap && sSceneData->EnvironmentMapActivated)
            sSceneData->EnvironmentMap->Render(sSceneData->ProjectionMatrix, sSceneData->ViewMatrix);
    }
}
