//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroRenderer2D.hpp"
#include "ElectroPipeline.hpp"
#include "ElectroVertexBuffer.hpp"
#include "ElectroIndexBuffer.hpp"
#include "ElectroConstantBuffer.hpp"
#include "ElectroTexture.hpp"
#include "ElectroShader.hpp"
#include "ElectroRenderCommand.hpp"
#include "ElectroRendererAPI.hpp"
#include "ElectroRendererAPISwitch.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Electro::Renderer2D
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
        Statistics Stats;
    };

    Renderer2DData data;

    void Init()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:   data.TextureShader = Shader::Create("Electro/assets/shaders/HLSL/Standard2D.hlsl"); break;
            case RendererAPI::API::OpenGL: data.TextureShader = Shader::Create("Electro/assets/shaders/GLSL/Standard2D.glsl"); break;
        }
        Vault::Submit<Shader>(data.TextureShader);

        data.TextureShader->Bind();

        ConstantBufferDesc desc;
        desc.Shader = data.TextureShader;
        desc.Name = "Data";
        desc.Data = nullptr;
        desc.Size = sizeof(ShaderConstantBuffer);
        desc.BindSlot = 0;
        desc.ShaderDomain = ShaderDomain::VERTEX;
        desc.Usage = DataUsage::DYNAMIC;
        data.CBuffer = ConstantBuffer::Create(desc);

        // Vertex Buffer
        VertexBufferLayout layout =
        {
            { ShaderDataType::Float3, "POSITION"     },
            { ShaderDataType::Float4, "COLOR"        },
            { ShaderDataType::Float2, "TEXCOORD"     },
            { ShaderDataType::Float,  "TEXINDEX"     },
            { ShaderDataType::Float,  "TILINGFACTOR" },
        };
        data.QuadVertexBufferBase = new QuadVertex[data.MaxVertices];
        data.QuadVertexBuffer = VertexBuffer::Create(data.MaxVertices * sizeof(QuadVertex), layout);

        // Index Buffer
        Uint* quadIndices = new Uint[data.MaxIndices];
        Uint offset = 0;
        for (Uint i = 0; i < data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, data.MaxIndices);
        quadIB->Bind();

        // Textures
        data.WhiteTexture = Texture2D::Create(1, 1);
        Uint whiteTextureData = 0xffffffff;
        data.WhiteTexture->SetData(&whiteTextureData, sizeof(Uint));

        int32_t samplers[data.MaxTextureSlots];
        for (Uint i = 0; i < data.MaxTextureSlots; i++)
            samplers[i] = i;

        // data.TextureShader->SetIntArray("u_Textures", samplers, data.MaxTextureSlots); //OpenGL only
        // Set first texture slot to 0
        data.TextureSlots[0] = data.WhiteTexture;

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                data.QuadVertexPositions[0] = {  0.5f,  0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[2] = { -0.5f, -0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f }; break;
            case RendererAPI::API::OpenGL:
                data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
                data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f }; break;
            default:
                E_INTERNAL_ASSERT("RendererAPI not supported!");
        }

        // Create the pipeline
        PipelineSpecification spec = {};
        spec.Shader = data.TextureShader;
        spec.IndexBuffer = quadIB;
        spec.VertexBuffer = data.QuadVertexBuffer;
        data.QuadPipeline = Pipeline::Create(spec);
        data.QuadPipeline->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        data.QuadPipeline->Bind();
        delete[] quadIndices;
    }

    void Shutdown()
    {
        delete[] data.QuadVertexBufferBase;
    }

    void BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

        data.TextureShader->Bind();
        data.CBuffer->SetData(&viewProj);
        data.QuadVertexBufferPtr = data.QuadVertexBufferBase;
        StartBatch();
    }

    void BeginScene(const EditorCamera& camera)
    {
        glm::mat4 viewProj = camera.GetViewProjection();
        data.TextureShader->Bind();
        data.CBuffer->SetData(&viewProj);
        data.QuadVertexBufferPtr = data.QuadVertexBufferBase;
        StartBatch();
    }

    void EndScene() { Flush(); }

    void StartBatch()
    {
        data.QuadIndexCount = 0;
        data.QuadVertexBufferPtr = data.QuadVertexBufferBase;
        data.TextureSlotIndex = 1;
    }

    void Flush()
    {
        if (data.QuadIndexCount == 0)
            return; // Nothing to draw

        Uint dataSize = (Uint)((byte*)data.QuadVertexBufferPtr - (byte*)data.QuadVertexBufferBase);
        data.QuadVertexBuffer->SetData(data.QuadVertexBufferBase, dataSize);

        // Bind textures
        for (Uint i = 0; i < data.TextureSlotIndex; i++)
            data.TextureSlots[i]->Bind(i, ShaderDomain::PIXEL);

        RenderCommand::DrawIndexed(data.QuadPipeline, data.QuadIndexCount);
        data.Stats.DrawCalls++;
    }

    void DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            data.QuadVertexBufferPtr->Position = transform * data.QuadVertexPositions[i];
            data.QuadVertexBufferPtr->Color = color;
            data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            data.QuadVertexBufferPtr->TexIndex = 0.0f;
            data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            data.QuadVertexBufferPtr++;
        }

        data.QuadIndexCount += 6;
        data.Stats.QuadCount++;
    }

    void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;

        /* [Spike] We are not doing a switch on RendererAPI::GetAPI() because, we don't want to do that every frame! [Spike] */
    #ifdef RENDERER_API_DX11
        constexpr glm::vec2 textureCoords[] = { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } };
    #elif defined RENDERER_API_OPENGL
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    #endif

        if (data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        float textureSlot = 0.0f;
        /* For each texture in the sData.TextureSlotIndex if *sData.TextureSlots[i]
         * is equal to the given texture grab that texture slot and set it */
        for (Uint i = 1; i < data.TextureSlotIndex; i++)
        {
            if (*data.TextureSlots[i] == *texture)
            {
                textureSlot = (float)i;
                break;
            }
        }

        if (textureSlot == 0.0f)
        {
            if (data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
            {
                Flush();
                StartBatch();
            }

            textureSlot = (float)data.TextureSlotIndex;
            data.TextureSlots[data.TextureSlotIndex] = texture;
            data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            data.QuadVertexBufferPtr->Position = transform * data.QuadVertexPositions[i];
            data.QuadVertexBufferPtr->Color = tintColor;
            data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            data.QuadVertexBufferPtr->TexIndex = textureSlot;
            data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            data.QuadVertexBufferPtr++;
        }

        data.QuadIndexCount += 6;
        data.Stats.QuadCount++;
    }

    void DrawDebugQuad(const glm::mat4& transform)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            Flush();
            StartBatch();
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            data.QuadVertexBufferPtr->Position = transform * data.QuadVertexPositions[i];
            data.QuadVertexBufferPtr->Color = { 0.0f, 1.0f, 0.0f, 1.0f };
            data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            data.QuadVertexBufferPtr->TexIndex = 0.0f;
            data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            data.QuadVertexBufferPtr++;
        }

        data.QuadIndexCount += 6;
        data.Stats.QuadCount++;
    }

    void UpdateStats() { memset(&data.Stats, 0, sizeof(Statistics)); }
    Statistics GetStats() { return data.Stats; }
}