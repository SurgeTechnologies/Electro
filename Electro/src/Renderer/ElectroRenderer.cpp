//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroRenderer.hpp"
#include "ElectroSceneRenderer.hpp"
#include "Interface/ElectroTexture.hpp"
#include "Renderer/ElectroRendererDebug.hpp"

namespace Electro
{
    static Uint sTotalDrawCalls;
    void Renderer::Init()
    {
        RenderCommand::Init();
        RendererDebug::Init();
        SceneRenderer::Init();
    }

    void Renderer::Shutdown()
    {
        RendererDebug::Shutdown();
        SceneRenderer::Shutdown();
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::Submit(Ref<Pipeline> pipeline, Uint size)
    {
        RenderCommand::DrawIndexed(pipeline, size);
        sTotalDrawCalls++;
    }

    void Renderer::DrawMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        mesh->GetPipeline()->Bind();
        mesh->GetPipeline()->BindSpecificationObjects();

        for (Submesh& submesh : mesh->GetSubmeshes())
        {
            mesh->GetMaterial()->Bind(submesh.MaterialIndex);
            submesh.CBuffer->SetDynamicData(&(transform * submesh.Transform));
            submesh.CBuffer->VSBind();
            RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
            sTotalDrawCalls++;
        }
    }

    void Renderer::DrawColliderMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        if(mesh)
        {
            auto& spec = mesh->GetPipeline()->GetSpecification();
            mesh->GetPipeline()->Bind();
            spec.VertexBuffer->Bind();
            spec.IndexBuffer->Bind();
            Vault::Get<Shader>("Collider.hlsl")->Bind();

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
