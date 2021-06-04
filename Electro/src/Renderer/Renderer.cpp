//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer.hpp"
#include "SceneRenderer.hpp"
#include "Renderer/RendererDebug.hpp"

namespace Electro
{
    static Uint sTotalDrawCalls;
    static Ref<Shader> sColliderShader;

    void Renderer::Init()
    {
        RendererDebug::Init();
        SceneRenderer::Init();
        sColliderShader = AssetManager::Get<Shader>("Collider.hlsl");
    }

    void Renderer::Shutdown()
    {
        SceneRenderer::Shutdown();
        RendererDebug::Shutdown();
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::ResizeBackbuffer(0, 0, width, height);
    }

    void Renderer::DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
    {
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

            submesh.CBuffer->SetDynamicData(&(transform * submesh.Transform));
            submesh.CBuffer->VSBind();
            RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            sTotalDrawCalls++;
        }
    }

    void Renderer::DrawColliderMesh(const Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        if(mesh)
        {
            const Ref<Pipeline>& pipeline = mesh->GetPipeline();

            sColliderShader->Bind();
            mesh->GetVertexBuffer()->Bind(pipeline->GetStride());
            mesh->GetIndexBuffer()->Bind();
            pipeline->Bind();

            RenderCommand::BeginWireframe();
            for (const Submesh& submesh : mesh->GetSubmeshes())
            {
                submesh.CBuffer->SetDynamicData(&(transform * submesh.Transform));
                submesh.CBuffer->VSBind();
                RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
                sTotalDrawCalls++;
            }
            RenderCommand::EndWireframe();
        }
    }

    RendererAPI::API Renderer::GetAPI()
    {
        return RendererAPI::GetAPI();
    }

    Uint Renderer::GetTotalDrawCallsCount()
    {
        return sTotalDrawCalls;
    }

    void Renderer::UpdateStatus()
    {
        sTotalDrawCalls = 0;
    }
}
