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

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual Uint GetCount() const = 0;
        static Ref<IndexBuffer> Create(void* indices, Uint count);
    };
}