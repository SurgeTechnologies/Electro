//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"

namespace Electro
{
    class VertexBufferLayout;
    class VertexBuffer;
    class IndexBuffer;
    class Pipeline;
    struct PipelineSpecification;

    class EDevice
    {
    public:
        static Ref<VertexBuffer> CreateVertexBuffer(Uint size, VertexBufferLayout layout);
        static Ref<VertexBuffer> CreateVertexBuffer(void* vertices, Uint size, VertexBufferLayout layout);
        static Ref<IndexBuffer> CreateIndexBuffer(void* indices, Uint count);
        static Ref<Pipeline> CreatePipeline(const PipelineSpecification& spec);
    };
}