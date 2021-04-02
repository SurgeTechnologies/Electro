//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroRenderer.hpp"
#include "ElectroSceneRenderer.hpp"

namespace Electro
{
    void Renderer::Init()
    {
        RenderCommand::Init();
        SceneRenderer::Init();
    }

    void Renderer::Shutdown()
    {
        SceneRenderer::Shutdown();
    }

    void Renderer::OnWindowResize(Uint width, Uint height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::Submit(Ref<Pipeline> pipeline, Uint size)
    {
        RenderCommand::DrawIndexed(pipeline, size);
    }

    void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
    {
        mesh->GetPipeline()->Bind();
        mesh->GetPipeline()->BindSpecificationObjects();

        for (Submesh& submesh : mesh->GetSubmeshes())
        {
            mesh->GetMaterial()->Bind(submesh.MaterialIndex);
            submesh.CBuffer->SetData(&(transform * submesh.Transform));
            RenderCommand::DrawIndexedMesh(submesh.IndexCount, submesh.BaseIndex, submesh.BaseVertex);
        }
    }

    RendererAPI::API Renderer::GetAPI()
    {
        return RendererAPI::GetAPI();
    }
}