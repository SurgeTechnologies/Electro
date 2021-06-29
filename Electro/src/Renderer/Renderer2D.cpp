//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer2D.hpp"
#include "Renderer.hpp"
#include "RenderCommand.hpp"

namespace Electro
{
    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct RendererDebugData
    {
        glm::mat4 ViewProjectionMatrix;

        static const Uint MaxLines = 20000;
        static const Uint MaxVertices = MaxLines * 2;
        Uint LineVertexCount = 0;

        Ref<Pipeline> LinePipeline;
        Ref<Shader> LineShader;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<ConstantBuffer> LineCBuffer;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;
    };
    static RendererDebugData sData;

    void Renderer2D::Init()
    {
        sData.LineVertexBuffer = VertexBuffer::Create(sData.MaxVertices * sizeof(LineVertex));
        sData.LineVertexBufferBase = new LineVertex[sData.MaxVertices];
        sData.LineShader  = Renderer::GetShader("Renderer2DLine");
        sData.LineCBuffer  = ConstantBuffer::Create(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
        sData.LinePipeline = Pipeline::Create();
        sData.LinePipeline->GenerateInputLayout(sData.LineShader);
    }

    void Renderer2D::Shutdown()
    {
        delete[] sData.LineVertexBufferBase;
    }

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
        memset(sData.LineVertexBufferBase, 0, sData.MaxVertices * sizeof(LineVertex));
        sData.LineVertexCount = 0;
        sData.ViewProjectionMatrix = camera.GetViewMatrix() * camera.GetProjection();
        StartBatch();
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        memset(sData.LineVertexBufferBase, 0, sData.MaxVertices * sizeof(LineVertex));
        sData.LineVertexCount = 0;
        sData.ViewProjectionMatrix = viewProjection;
        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Linelist);
        Flush();
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);
    }

    void Renderer2D::Flush()
    {
        if (sData.LineVertexCount == 0)
            return;

        const Uint dataSize = static_cast<Uint>(reinterpret_cast<uint8_t*>(sData.LineVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.LineVertexBufferBase));
        sData.LineShader->Bind();
        sData.LineVertexBuffer->SetData(sData.LineVertexBufferBase, dataSize);
        sData.LineVertexBuffer->Bind(sData.LinePipeline->GetStride());
        sData.LineCBuffer->SetDynamicData(&sData.ViewProjectionMatrix);
        sData.LinePipeline->Bind();
        RenderCommand::Draw(sData.LineVertexCount);
        sData.LinePipeline->Unbind();
    }

    void Renderer2D::SubmitLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color)
    {
        if (sData.LineVertexCount >= RendererDebugData::MaxVertices)
            NextBatch();

        sData.LineVertexBufferPtr->Position = p1;
        sData.LineVertexBufferPtr->Color = color;
        sData.LineVertexBufferPtr++;

        sData.LineVertexBufferPtr->Position = p2;
        sData.LineVertexBufferPtr->Color = color;
        sData.LineVertexBufferPtr++;

        sData.LineVertexCount += 2;
    }

    void Renderer2D::SubmitAABB(const BoundingBox& aabb, const glm::mat4& transform, const glm::vec4& color)
    {
        glm::vec4 corners[8] =
        {
            transform * glm::vec4(aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f),
            transform * glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f),
            transform * glm::vec4(aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f),
            transform * glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f),

            transform * glm::vec4(aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f),
            transform * glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f),
            transform * glm::vec4(aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f),
            transform * glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f)
        };

        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[(i + 1) % 4], color);
        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);
        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[i + 4], color);
    }

    void Renderer2D::SubmitAABB(glm::vec4* corners, const glm::mat4& transform, const glm::vec4& color)
    {
        corners[0] = transform * corners[0];
        corners[1] = transform * corners[1];
        corners[3] = transform * corners[3];
        corners[3] = transform * corners[3];
        corners[4] = transform * corners[4];
        corners[5] = transform * corners[5];
        corners[6] = transform * corners[6];
        corners[7] = transform * corners[7];

        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[(i + 1) % 4], color);
        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);
        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[i + 4], color);
    }

    void Renderer2D::StartBatch()
    {
        sData.LineVertexBufferPtr = sData.LineVertexBufferBase;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }
}
