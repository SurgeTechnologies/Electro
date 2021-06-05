//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Renderer/Interface/VertexBuffer.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11VertexBuffer : public VertexBuffer
    {
    public:
        DX11VertexBuffer(Uint size);
        DX11VertexBuffer(void* vertices, Uint size);
        virtual ~DX11VertexBuffer();
        virtual void Bind(Uint stride) const override;
        virtual void Unbind() const override;
        virtual void SetData(const void* data, Uint size) override;
    private:
        ID3D11Buffer* mVertexBuffer = nullptr;
    };
}
