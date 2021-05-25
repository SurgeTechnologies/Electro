//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "RendererDebug.hpp"
#include "Factory.hpp"
#include "Interface/Pipeline.hpp"
#include "Interface/VertexBuffer.hpp"
#include "Interface/ConstantBuffer.hpp"
#include "Interface/Shader.hpp"
#include "RenderCommand.hpp"

namespace Electro
{
    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct Line
    {
        glm::vec3 P1;
        glm::vec3 P2;
    };

    struct RendererDebugData
    {
        static const Uint MaxLines = 20000;
        static const Uint MaxVertices = MaxLines * 2;

        Uint LineVertexCount = 0;

        Ref<Shader> DebugShader;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<Pipeline> LinePipeline;
        Ref<ConstantBuffer> LineCBuffer;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;
        bool ShowCameraFrustum = true;

        //Grid
        Vector<Line> GridPositions;
        bool ShowGrid = true;
    };

    static RendererDebugData sData;

    Scope<RendererDebug::SceneData> RendererDebug::mSceneData = CreateScope<RendererDebug::SceneData>();

    void RendererDebug::Init()
    {
        PipelineSpecification spec;
        VertexBufferLayout layout =
        {
            { ShaderDataType::Float3, "POSITION" },
            { ShaderDataType::Float4, "COLOR" },
        };

        sData.LineVertexBuffer = Factory::CreateVertexBuffer(sData.MaxVertices * sizeof(LineVertex), layout);
        sData.LineVertexBufferBase = new LineVertex[sData.MaxVertices];
        sData.DebugShader = Factory::CreateShader("Electro/assets/shaders/HLSL/Debug.hlsl");
        sData.LineCBuffer = Factory::CreateConstantBuffer(sizeof(SceneData), 0, DataUsage::DYNAMIC);
        spec.VertexBuffer = sData.LineVertexBuffer;
        spec.Shader = sData.DebugShader;
        spec.IndexBuffer = nullptr;
        sData.LinePipeline = Factory::CreatePipeline(spec);

        //Grid
        int count = 11;
        float length = 10;
        for (float j = 0; j < count; ++j)
        {
            sData.GridPositions.push_back({ glm::vec3(j, 0, -length), glm::vec3(j, 0, length) });
            sData.GridPositions.push_back({ glm::vec3(-length, 0, j), glm::vec3(length, 0, j) });
        }

        for (float j = 0; j > -count; --j)
        {
            sData.GridPositions.push_back({ glm::vec3(j, 0, -length), glm::vec3(j, 0, length) });
            sData.GridPositions.push_back({ glm::vec3(-length, 0, j), glm::vec3(length, 0, j) });
        }
    }

    void RendererDebug::Shutdown()
    {
        delete[] sData.LineVertexBufferBase;
    }

    void RendererDebug::BeginScene(const EditorCamera& camera)
    {
        memset(sData.LineVertexBufferBase, 0, sData.MaxVertices * sizeof(LineVertex));
        sData.LineVertexCount = 0;

        mSceneData->ViewProjectionMatrix = camera.GetViewMatrix() * camera.GetProjection();
        sData.DebugShader->Bind();
        sData.LineCBuffer->SetDynamicData(&mSceneData);
        sData.LineVertexBuffer->Bind();
        StartBatch();
    }

    void RendererDebug::EndScene()
    {
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Linelist);
        Flush();
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);
    }

    void RendererDebug::Flush()
    {
        if (sData.LineVertexCount == 0)
            return;

        Uint dataSize = (Uint)((uint8_t*)sData.LineVertexBufferPtr - (uint8_t*)sData.LineVertexBufferBase);
        sData.LineVertexBuffer->SetData(sData.LineVertexBufferBase, dataSize);
        sData.LinePipeline->Bind();
        RenderCommand::Draw(sData.LineVertexCount);
    }

    void RendererDebug::SubmitCameraFrustum(SceneCamera& camera, glm::mat4& transform, glm::vec3& pos)
    {
        if (sData.ShowCameraFrustum)
        {
            const glm::mat4 inv = (transform * glm::inverse(camera.GetProjection()));
            glm::vec4 f[8] =
            {
                //Near face
                {  1.0f,  1.0f, -1.0f, 1.0f },
                { -1.0f,  1.0f, -1.0f, 1.0f },
                {  1.0f, -1.0f, -1.0f, 1.0f },
                { -1.0f, -1.0f, -1.0f, 1.0f },

                //Far face
                {  1.0f,  1.0f, 1.0f, 1.0f },
                { -1.0f,  1.0f, 1.0f, 1.0f },
                {  1.0f, -1.0f, 1.0f, 1.0f },
                { -1.0f, -1.0f, 1.0f, 1.0f },
            };

            glm::vec3 v[8];
            for (int i = 0; i < 8; i++)
            {
                const glm::vec4 ff = inv * f[i];
                v[i].x = ff.x / ff.w;
                v[i].y = ff.y / ff.w;
                v[i].z = ff.z / ff.w;
            }

            RendererDebug::SubmitLine(v[0], v[1]);
            RendererDebug::SubmitLine(v[0], v[2]);
            RendererDebug::SubmitLine(v[3], v[1]);
            RendererDebug::SubmitLine(v[3], v[2]);

            RendererDebug::SubmitLine(v[4], v[5]);
            RendererDebug::SubmitLine(v[4], v[6]);
            RendererDebug::SubmitLine(v[7], v[5]);
            RendererDebug::SubmitLine(v[7], v[6]);

            RendererDebug::SubmitLine(v[0], v[4]);
            RendererDebug::SubmitLine(v[1], v[5]);
            RendererDebug::SubmitLine(v[3], v[7]);
            RendererDebug::SubmitLine(v[2], v[6]);
        }
    }

    void RendererDebug::SubmitLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color)
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

    void RendererDebug::DrawAABB(const BoundingBox& aabb, const glm::mat4& transform, const glm::vec4& color)
    {
        glm::vec4 corners[8] =
        {
            transform * glm::vec4{ aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
            transform * glm::vec4{ aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4{ aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4{ aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

            transform * glm::vec4{ aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
            transform * glm::vec4{ aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4{ aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4{ aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
        };

        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[(i + 1) % 4], color);

        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

        for (Uint i = 0; i < 4; i++)
            SubmitLine(corners[i], corners[i + 4], color);
    }

    void RendererDebug::StartBatch()
    {
        sData.LineVertexBufferPtr = sData.LineVertexBufferBase;
    }

    void RendererDebug::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void RendererDebug::RenderGrid()
    {
        if (sData.ShowGrid)
        {
            for (auto& pos : sData.GridPositions)
                RendererDebug::SubmitLine(pos.P1, pos.P2, { 0.2f, 0.7f, 0.2f, 1.0f });
        }
    }

    Pair<bool*, bool*> RendererDebug::GetToggles()
    {
        return { &sData.ShowGrid, &sData.ShowCameraFrustum };
    }
}
