//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderer2D.hpp"
#include "Asset/AssetManager.hpp"
#include "Factory.hpp"
#include "Interface/Pipeline.hpp"
#include "Interface/VertexBuffer.hpp"
#include "Interface/IndexBuffer.hpp"
#include "Interface/ConstantBuffer.hpp"
#include "Interface/Texture.hpp"
#include "Interface/Shader.hpp"
#include "RenderCommand.hpp"
#include "RendererAPI.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Electro
{
    struct ShaderConstantBuffer
    {
        glm::mat4 ViewProjection;
    };

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct Renderer2DData
    {
        static const Uint MaxQuads = 20000;
        static const Uint MaxVertices = MaxQuads * 4;
        static const Uint MaxIndices = MaxQuads * 6;
        static const Uint MaxTextureSlots = 32;

        Ref<Pipeline> QuadPipeline;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;
        Ref<ConstantBuffer> CBuffer;

        Uint QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        Uint TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];
        Renderer2D::Statistics Stats;
    };

    static Renderer2DData sData;

    void Renderer2D::Init()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:   sData.TextureShader = Factory::CreateShader("Electro/assets/shaders/HLSL/Standard2D.hlsl"); break;
            case RendererAPI::API::OpenGL: sData.TextureShader = Factory::CreateShader("Electro/assets/shaders/GLSL/Standard2D.glsl"); break;
        }

        sData.TextureShader->Bind();

        //Set up the Constant Buffer for Renderer2D
        sData.CBuffer = Factory::CreateConstantBuffer(sizeof(ShaderConstantBuffer), 0, DataUsage::DYNAMIC);

        // Vertex Buffer
        VertexBufferLayout layout =
        {
            { ShaderDataType::Float3, "POSITION"     },
            { ShaderDataType::Float4, "COLOR"        },
            { ShaderDataType::Float2, "TEXCOORD"     },
            { ShaderDataType::Float,  "TEXINDEX"     },
            { ShaderDataType::Float,  "TILINGFACTOR" },
        };
        sData.QuadVertexBufferBase = new QuadVertex[sData.MaxVertices];
        sData.QuadVertexBuffer = Factory::CreateVertexBuffer(sData.MaxVertices * sizeof(QuadVertex), layout);

        // Index Buffer
        Uint* quadIndices = new Uint[sData.MaxIndices];
        Uint offset = 0;
        for (Uint i = 0; i < sData.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        Ref<IndexBuffer> quadIB = Factory::CreateIndexBuffer(quadIndices, sData.MaxIndices);
        quadIB->Bind();

        // Textures
        sData.WhiteTexture = Factory::CreateTexture2D(1, 1);
        Uint whiteTextureData = 0xffffffff;
        sData.WhiteTexture->SetData(&whiteTextureData, sizeof(Uint));

        int32_t samplers[sData.MaxTextureSlots];
        for (Uint i = 0; i < sData.MaxTextureSlots; i++)
            samplers[i] = i;

        // data.TextureShader->SetIntArray("u_Textures", samplers, data.MaxTextureSlots); //OpenGL only
        // Set first texture slot to 0
        sData.TextureSlots[0] = sData.WhiteTexture;

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sData.QuadVertexPositions[0] = {  0.5f,  0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[2] = { -0.5f, -0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f }; break;
            case RendererAPI::API::OpenGL:
                sData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
                sData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f }; break;
            default:
                E_INTERNAL_ASSERT("RendererAPI not supported!");
        }

        // Create the pipeline
        PipelineSpecification spec = {};
        spec.Shader = sData.TextureShader;
        spec.IndexBuffer = quadIB;
        spec.VertexBuffer = sData.QuadVertexBuffer;
        sData.QuadPipeline = Factory::CreatePipeline(spec);
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        sData.QuadPipeline->Bind();
        delete[] quadIndices;
    }

    void Renderer2D::Shutdown()
    {
        delete[] sData.QuadVertexBufferBase;
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

        sData.TextureShader->Bind();
        sData.CBuffer->SetDynamicData(&viewProj);
        sData.CBuffer->VSBind();
        sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
        StartBatch();
    }

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
        glm::mat4 viewProj = camera.GetViewProjection();
        sData.TextureShader->Bind();
        sData.CBuffer->SetDynamicData(&viewProj);
        sData.CBuffer->VSBind();
        sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
        StartBatch();
    }

    void Renderer2D::EndScene() { Flush(); }

    void Renderer2D::StartBatch()
    {
        sData.QuadIndexCount = 0;
        sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
        sData.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush()
    {
        if (sData.QuadIndexCount == 0)
            return; // Nothing to draw

        Uint dataSize = (Uint)((byte*)sData.QuadVertexBufferPtr - (byte*)sData.QuadVertexBufferBase);
        sData.QuadVertexBuffer->SetData(sData.QuadVertexBufferBase, dataSize);

        // Bind textures
        for (Uint i = 0; i < sData.TextureSlotIndex; i++)
            sData.TextureSlots[i]->PSBind(i);

        RenderCommand::DrawIndexed(sData.QuadPipeline, sData.QuadIndexCount);
        sData.Stats.DrawCalls++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
            sData.QuadVertexBufferPtr->Color = color;
            sData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            sData.QuadVertexBufferPtr->TexIndex = 0.0f;
            sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            sData.QuadVertexBufferPtr++;
        }

        sData.QuadIndexCount += 6;
        sData.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } };

        if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        float textureSlot = 0.0f;
        /* For each texture in the sData.TextureSlotIndex if *sData.TextureSlots[i]
         * is equal to the given texture grab that texture slot and set it */
        for (Uint i = 1; i < sData.TextureSlotIndex; i++)
        {
            if (*sData.TextureSlots[i] == *texture)
            {
                textureSlot = (float)i;
                break;
            }
        }

        if (textureSlot == 0.0f)
        {
            if (sData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
            {
                Flush();
                StartBatch();
            }

            textureSlot = (float)sData.TextureSlotIndex;
            sData.TextureSlots[sData.TextureSlotIndex] = texture;
            sData.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
            sData.QuadVertexBufferPtr->Color = tintColor;
            sData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            sData.QuadVertexBufferPtr->TexIndex = textureSlot;
            sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            sData.QuadVertexBufferPtr++;
        }

        sData.QuadIndexCount += 6;
        sData.Stats.QuadCount++;
    }

    void Renderer2D::DrawDebugQuad(const glm::mat4& transform)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
            sData.QuadVertexBufferPtr->Color = { 0.0f, 1.0f, 0.0f, 1.0f };
            sData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            sData.QuadVertexBufferPtr->TexIndex = 0.0f;
            sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            sData.QuadVertexBufferPtr++;
        }

        sData.QuadIndexCount += 6;
        sData.Stats.QuadCount++;
    }

    void Renderer2D::UpdateStats() { memset(&sData.Stats, 0, sizeof(Renderer2D::Statistics)); }
    Renderer2D::Statistics Renderer2D::GetStats() { return sData.Stats; }
}
