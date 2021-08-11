//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DebugPass.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"

namespace Electro
{
    void DebugPass::Init(RendererData* rendererData)
    {
        mRendererData = rendererData;
        mData.SolidColorShader = Renderer::GetShader("SolidColor");
        mData.OutlineShader = Renderer::GetShader("Outline");
        mData.GridShader = Renderer::GetShader("Grid");

        Uint width = 1280;
        Uint height = 720;

        {   // Outline Texture
            RenderbufferSpecification fbSpec;
            fbSpec.Attachments = { RenderBufferTextureFormat::RGBA32F };
            fbSpec.Width = width;
            fbSpec.Height = height;
            fbSpec.SwapChainTarget = false;
            fbSpec.DebugName = "Outline Texture";
            mData.OutlineRenderBuffer = Renderbuffer::Create(fbSpec);
        }
    }

    void DebugPass::Update()
    {
        const Ref<Renderbuffer>& targetRenderbuffer = mRendererData->GeometryBuffer;
        targetRenderbuffer->Bind();

        mRendererData->SceneCBuffer->VSBind();
        mRendererData->SceneCBuffer->SetDynamicData(&mRendererData->ViewProjectionMatrix);
        Renderer2D::BeginScene(mRendererData->ViewProjectionMatrix);
        if (mData.ShowCameraFrustum)
        {
            glm::mat4 cameraView;
            glm::mat4 cameraProjection;
            {
                auto view = mRendererData->SceneContext->GetRegistry().view<TransformComponent, CameraComponent>();
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
        if (mData.ShowAABB)
        {
            for (const DrawCommand& drawCmd : mRendererData->MeshDrawList)
            {
                const Vector<Submesh>& submeshes = drawCmd.GetMesh()->GetSubmeshes();
                for (const Submesh& submesh : submeshes)
                {
                    Renderer2D::SubmitAABB(submesh.BoundingBox, drawCmd.GetTransform() * submesh.Transform);
                }
            }
        }
        Renderer2D::EndScene();

        // Outline
        if (!mRendererData->OutlineDrawList.empty())
        {
            mData.OutlineRenderBuffer->Bind();
            mData.OutlineRenderBuffer->Clear({ 0.1f, 0.1f, 0.1f, 1.0f });
            RenderCommand::DisableDepth();
            for (const DrawCommand& drawCmd : mRendererData->OutlineDrawList)
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

                    mRendererData->TransformCBuffer->VSBind();
                    mRendererData->TransformCBuffer->SetDynamicData(&(drawCmd.GetTransform() * submesh.Transform));
                    mData.SolidColorShader->Bind();
                    mRendererData->SolidColorCBuffer->PSBind();
                    mRendererData->SolidColorCBuffer->SetDynamicData(&color);
                    RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                }

                pipeline->Unbind();
            }
            mData.OutlineRenderBuffer->Unbind();

            targetRenderbuffer->Bind(); // Bind the target again as other outline renderbuffer got bound
            mData.OutlineShader->Bind();
            mData.OutlineRenderBuffer->BindColorBuffer(0, 0, ShaderDomain::PIXEL);
            Renderer::RenderFullscreenQuad();
            mData.OutlineRenderBuffer->UnbindBuffer(0, ShaderDomain::PIXEL);
            RenderCommand::EnableDepth();
        }
        if (mData.ShowGrid)
        {
            mData.GridShader->Bind();
            RenderCommand::Draw(3); // Render Full screen Quad, but with depth enabled
        }
        // Collider
        {
            for (const ColliderDrawCommand& drawCmd : mRendererData->ColliderDrawList)
            {
                if (drawCmd.Show)
                {
                    const Ref<Mesh>& mesh = drawCmd.Mesh;
                    const Ref<Pipeline>& pipeline = mesh->GetPipeline();
                    glm::vec4 color = { 0.1f, 0.9f, 0.1f, 1.0f };

                    pipeline->Bind();
                    mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
                    mesh->GetIndexBuffer()->Bind();
                    mData.SolidColorShader->Bind();
                    mRendererData->SolidColorCBuffer->PSBind();
                    mRendererData->SolidColorCBuffer->SetDynamicData(&color);

                    RenderCommand::BeginWireframe();
                    for (const Submesh& submesh : mesh->GetSubmeshes())
                    {
                        mRendererData->TransformCBuffer->VSBind();
                        mRendererData->TransformCBuffer->SetDynamicData(&(drawCmd.Transform * submesh.Transform));
                        RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                    }
                    RenderCommand::EndWireframe();
                    pipeline->Unbind();
                }
            }
        }
        targetRenderbuffer->Unbind();
    }

    void DebugPass::Shutdown()
    {
    }
}
