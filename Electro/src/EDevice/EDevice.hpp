//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"

namespace Electro
{
    class  VertexBufferLayout;
    class  VertexBuffer;
    class  IndexBuffer;
    class  Pipeline;
    class  Shader;
    class  ConstantBuffer;
    class  Framebuffer;
    class  Texture2D;
    class  Cubemap;
    struct FramebufferSpecification;
    struct PipelineSpecification;
    struct DeviceStatus;
    enum class ShaderDomain;
    enum class DataUsage;

    class EDevice
    {
    public:
        static Ref<VertexBuffer> CreateVertexBuffer(Uint size, VertexBufferLayout layout);
        static Ref<VertexBuffer> CreateVertexBuffer(void* vertices, Uint size, VertexBufferLayout layout);
        static Ref<IndexBuffer> CreateIndexBuffer(void* indices, Uint count);
        static Ref<Framebuffer> CreateFramebuffer(const FramebufferSpecification& spec);
        static Ref<Shader> CreateShader(const String& filepath);
        static Ref<ConstantBuffer> CreateConstantBuffer(Uint size, Uint bindSlot, ShaderDomain shaderDomain, DataUsage usage);
        static Ref<Pipeline> CreatePipeline(const PipelineSpecification& spec);
        static Ref<Texture2D> CreateTexture2D(Uint width, Uint height);
        static Ref<Texture2D> CreateTexture2D(const String& path, bool srgb = false, bool flipped = false);
        static Ref<Cubemap> CreateCubemap(const String& path);

        DeviceStatus GetEDeviceStatus();
    };
}