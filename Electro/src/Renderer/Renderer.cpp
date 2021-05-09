//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer.hpp"
#include "SceneRenderer.hpp"
#include "Interface/Texture.hpp"
#include "Renderer/RendererDebug.hpp"

namespace Electro
{
    static Uint sTotalDrawCalls;
    void Renderer::Init()
    {
        RendererDebug::Init();
        SceneRenderer::Init();
    }

    void Renderer::Shutdown()
    {
        SceneRenderer::Shutdown();
        RendererDebug::Shutdown();
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    const void Renderer::Submit(Ref<Pipeline>& pipeline, Uint size)
    {
        RenderCommand::DrawIndexed(pipeline, size);
        sTotalDrawCalls++;
    }

    const void Renderer::DrawMesh(Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        mesh->GetPipeline()->Bind();
        mesh->GetPipeline()->BindSpecificationObjects();

        Vector<Ref<Material>>& materials = mesh->GetMaterials();
        for (Uint i = 0; i < mesh->GetSubmeshes().size(); i++)
        {
            Submesh& submesh = mesh->GetSubmeshes()[i];
            materials[submesh.MaterialIndex]->Bind();

            submesh.CBuffer->SetDynamicData(&(transform * submesh.Transform));
            submesh.CBuffer->VSBind();
            RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            sTotalDrawCalls++;
        }
    }

    const void Renderer::DrawColliderMesh(Ref<Mesh>& mesh, const glm::mat4& transform)
    {
        if(mesh)
        {
            auto& spec = mesh->GetPipeline()->GetSpecification();
            mesh->GetPipeline()->Bind();
            spec.VertexBuffer->Bind();
            spec.IndexBuffer->Bind();
            AssetManager::Get<Shader>("Collider.hlsl")->Bind();

            RenderCommand::BeginWireframe();
            for (Submesh& submesh : mesh->GetSubmeshes())
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
