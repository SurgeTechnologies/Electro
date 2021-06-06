//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/Log.hpp"

namespace Electro
{
    class VertexBuffer : public IElectroRef
    {
    public:
        virtual ~VertexBuffer() = default;

        // Binds the VertexBuffer to the pipeline
        virtual void Bind(Uint stride) const = 0;

        // Unbinds the VertexBuffer from the pipeline, this doesnt give 100% gurantee that it will Unbind it, may not work for all graphics API
        virtual void Unbind() const = 0;

        // Sets the actual raw data of the vertex buffer, useful for dynamic vertex buffer
        // Make sure that the correct Vertex Buffer is bound before calling this
        virtual void SetData(const void* data, Uint size) = 0;

        static Ref<VertexBuffer> Create(Uint size);
        static Ref<VertexBuffer> Create(void* vertices, Uint size);
    };
}
