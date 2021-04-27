//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroRendererDebug.hpp"
#include "EDevice/EDevice.hpp"
#include "Renderer/Interface/ElectroPipeline.hpp"
#include "Renderer/Interface/ElectroVertexBuffer.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "Renderer/Interface/ElectroShader.hpp"
#include "Renderer/ElectroRenderCommand.hpp"

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

        //Grid
        Vector<Line> GridPositions;
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

        sData.LineVertexBuffer = EDevice::CreateVertexBuffer(sData.MaxVertices * sizeof(LineVertex), layout);
        sData.LineVertexBufferBase = new LineVertex[sData.MaxVertices];
        sData.DebugShader = EDevice::CreateShader("Electro/assets/shaders/HLSL/Debug.hlsl");
        sData.LineCBuffer = EDevice::CreateConstantBuffer(sizeof(SceneData), 0, DataUsage::DYNAMIC);
        spec.VertexBuffer = sData.LineVertexBuffer;
        spec.Shader = sData.DebugShader;
        spec.IndexBuffer = nullptr;
        sData.LinePipeline = EDevice::CreatePipeline(spec);

        //Grid
        int count = 10;

        float length = (float)count - 0.5;
        for (float j = 0.5; j <= count; ++j)
        {
            sData.GridPositions.push_back({ glm::vec3(j, 0, -length), glm::vec3(j, 0, length) });
            sData.GridPositions.push_back({ glm::vec3(-length, 0, j), glm::vec3(length, 0, j) });
        }
        for (float j = 0.5; j >= -count; --j)
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
        Flush();
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
        glm::mat4 inv = (transform * glm::inverse(camera.GetProjection()));

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

        glm::vec3 v[8u];
        for (int i = 0; i < 8; i++)
        {
            glm::vec4 ff = inv * f[i];
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

    void RendererDebug::SubmitLine(glm::vec3& p1, glm::vec3& p2)
    {
        if (sData.LineVertexCount >= RendererDebugData::MaxVertices)
            NextBatch();

        sData.LineVertexBufferPtr->Position = p1;
        sData.LineVertexBufferPtr->Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        sData.LineVertexBufferPtr++;

        sData.LineVertexBufferPtr->Position = p2;
        sData.LineVertexBufferPtr->Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        sData.LineVertexBufferPtr++;

        sData.LineVertexCount += 2;
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
        for (auto& pos : sData.GridPositions)
            RendererDebug::SubmitLine(pos.P1, pos.P2);
    }
}
