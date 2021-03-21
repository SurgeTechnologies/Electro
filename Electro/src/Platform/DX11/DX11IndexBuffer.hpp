//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroIndexBuffer.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11IndexBuffer : public IndexBuffer
    {
    public:
        DX11IndexBuffer(void* indices, Uint count);
        virtual ~DX11IndexBuffer();

        void Bind() const override;
        void Unbind() const override;

        Uint GetCount() const override { return mCount; }
    private:
        ID3D11Buffer* mIndexBuffer;
        Uint mCount;
    };
}
