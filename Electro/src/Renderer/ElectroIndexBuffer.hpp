//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"

namespace Electro
{
    class IndexBuffer : public IElectroRef
    {
    public:
        virtual ~IndexBuffer() = default;

        //Binds the Index buffer to the pipeline
        virtual void Bind() const = 0;

        //Unbinds the Index buffer from the pipeline, this may not work for all graphics API
        virtual void Unbind() const = 0;

        //Gets the number of elements in the index buffer
        virtual Uint GetCount() const = 0;

        //Creates an index buffer with STATIC usage
        static Ref<IndexBuffer> Create(void* indices, Uint count);
    };
}