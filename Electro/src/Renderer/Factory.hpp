//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"

namespace Electro
{
    class  VertexBufferLayout;
    class  VertexBuffer;
    class  IndexBuffer;
    class  Pipeline;
    class  Shader;
    class  ConstantBuffer;
    class  Framebuffer;
    class  Mesh;
    class  Texture2D;
    class  Cubemap;
    class  EnvironmentMap;
    class  Material;
    class  PhysicsMaterial;
    struct FramebufferSpecification;
    struct PipelineSpecification;
    enum class ShaderDomain;
    enum class DataUsage;

    class Factory
    {
    public:
        static Ref<VertexBuffer> CreateVertexBuffer(Uint size, VertexBufferLayout layout);
        static Ref<VertexBuffer> CreateVertexBuffer(void* vertices, Uint size, VertexBufferLayout layout);
        static Ref<IndexBuffer> CreateIndexBuffer(void* indices, Uint count);
        static Ref<Framebuffer> CreateFramebuffer(const FramebufferSpecification& spec);
        static Ref<Shader> CreateShader(const String& filepath);
        static Ref<ConstantBuffer> CreateConstantBuffer(Uint size, Uint bindSlot, DataUsage usage);
        static Ref<Pipeline> CreatePipeline(const PipelineSpecification& spec);
        static Ref<Texture2D> CreateTexture2D(Uint width, Uint height);
        static Ref<Texture2D> CreateTexture2D(const String& path, bool srgb = false);
        static Ref<Cubemap> CreateCubemap(const String& path);
        static Ref<EnvironmentMap> CreateEnvironmentMap(const String& path);
        static Ref<Mesh> CreateMesh(const String& path);
        static Ref<Material> CreateMaterial(const Ref<Shader>& shader, const String& nameInShader, const String& pathOrName = "");
        static Ref<PhysicsMaterial> CreatePhysicsMaterial(const String& path);
    };
}
