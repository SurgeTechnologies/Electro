//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "RendererDebug.hpp"
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
            { ShaderDataType::Float3, "DEBUG_POSITION" },
            { ShaderDataType::Float4, "DEBUG_COLOR" },
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

    void RendererDebug::BeginScene(const glm::mat4& viewProjection)
    {
        memset(sData.LineVertexBufferBase, 0, sData.MaxVertices * sizeof(LineVertex));
        sData.LineVertexCount = 0;

        mSceneData->ViewProjectionMatrix = viewProjection;
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

        const Uint dataSize = static_cast<Uint>(reinterpret_cast<uint8_t*>(sData.LineVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.LineVertexBufferBase));
        sData.LineVertexBuffer->SetData(sData.LineVertexBufferBase, dataSize);
        sData.LinePipeline->Bind();
        RenderCommand::Draw(sData.LineVertexCount);
    }

    void RendererDebug::SubmitCameraFrustum(SceneCamera& camera, const glm::mat4& transform)
    {
        if (sData.ShowCameraFrustum)
        {
            glm::vec3* v = camera.GetFrustumPoints(transform);
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

    void RendererDebug::SubmitCameraFrustum(glm::vec4* points, const glm::mat4& transform, const glm::vec4& color)
    {
        //The size of points must be 8
        for(Uint i = 0; i < 8; i++)
            points[i] = transform * points[i];

        SubmitLine(points[0], points[1], color);
        SubmitLine(points[0], points[2], color);
        SubmitLine(points[3], points[1], color);
        SubmitLine(points[3], points[2], color);

        SubmitLine(points[4], points[5], color);
        SubmitLine(points[4], points[6], color);
        SubmitLine(points[7], points[5], color);
        SubmitLine(points[7], points[6], color);

        SubmitLine(points[0], points[4], color);
        SubmitLine(points[1], points[5], color);
        SubmitLine(points[3], points[7], color);
        SubmitLine(points[2], points[6], color);
    }

    void RendererDebug::SubmitCameraFrustum(glm::vec3* points, const glm::vec4& color)
    {
        SubmitLine(points[0], points[1], color);
        SubmitLine(points[0], points[2], color);
        SubmitLine(points[3], points[1], color);
        SubmitLine(points[3], points[2], color);

        SubmitLine(points[4], points[5], color);
        SubmitLine(points[4], points[6], color);
        SubmitLine(points[7], points[5], color);
        SubmitLine(points[7], points[6], color);

        SubmitLine(points[0], points[4], color);
        SubmitLine(points[1], points[5], color);
        SubmitLine(points[3], points[7], color);
        SubmitLine(points[2], points[6], color);
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

    void RendererDebug::DrawAABB(glm::vec4* corners, const glm::mat4& transform, const glm::vec4& color)
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

    void RendererDebug::SubmitOrthogonalProjection(const glm::mat4& viewProjection)
    {
        glm::mat4 inv = glm::inverse(viewProjection);

        glm::vec4 ndc[8] =
        {
            // near face
            { 1, 1, -1, 1.f },
            { -1, 1, -1, 1.f },
            { 1, -1, -1, 1.f },
            { -1, -1, -1, 1.f },

            // far face
            { 1, 1, 1, 1.f },
            { -1, 1, 1, 1.f },
            { 1, -1, 1, 1.f },
            { -1, -1, 1, 1.f },
        };

        glm::vec3 v[8];
        for (int i = 0; i < 8; i++)
        {
            glm::vec4 wc = inv * ndc[i];
            v[i].x = wc.x / wc.w;
            v[i].y = wc.y / wc.w;
            v[i].z = wc.z / wc.w;
        }

        SubmitLine(v[0], v[1], { 0.0f, 0.0f, 1.0f, 1.0f });
        SubmitLine(v[0], v[2], { 0.0f, 0.0f, 1.0f, 1.0f });
        SubmitLine(v[3], v[1], { 0.0f, 0.0f, 1.0f, 1.0f });
        SubmitLine(v[3], v[2], { 0.0f, 0.0f, 1.0f, 1.0f });

        SubmitLine(v[4], v[5], { 0.0f, 1.0f, 0.0f, 1.0f });
        SubmitLine(v[4], v[6], { 0.0f, 1.0f, 0.0f, 1.0f });
        SubmitLine(v[7], v[5], { 0.0f, 1.0f, 0.0f, 1.0f });
        SubmitLine(v[7], v[6], { 0.0f, 1.0f, 0.0f, 1.0f });

        SubmitLine(v[0], v[4], { 1.0f, 0.0f, 0.0f, 1.0f });
        SubmitLine(v[1], v[5], { 1.0f, 0.0f, 0.0f, 1.0f });
        SubmitLine(v[3], v[7], { 1.0f, 0.0f, 0.0f, 1.0f });
        SubmitLine(v[2], v[6], { 1.0f, 0.0f, 0.0f, 1.0f });
    }
}
