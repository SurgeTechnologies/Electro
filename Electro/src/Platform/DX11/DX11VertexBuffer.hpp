//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Renderer/Interface/ElectroVertexBuffer.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11VertexBuffer : public VertexBuffer
    {
    public:
        DX11VertexBuffer(Uint size, VertexBufferLayout layout);
        DX11VertexBuffer(void* vertices, Uint size, VertexBufferLayout layout);
        virtual ~DX11VertexBuffer();
        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(const void* data, Uint size) override;

        virtual void SetLayout(const VertexBufferLayout& layout) override { mLayout = layout; }
        virtual const VertexBufferLayout& GetLayout() const override { return mLayout; }
    private:
        VertexBufferLayout mLayout;
        ID3D11Buffer* mVertexBuffer = nullptr;
    };
}