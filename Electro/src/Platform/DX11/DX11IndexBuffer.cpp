//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11IndexBuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    DX11IndexBuffer::DX11IndexBuffer(void* indices, Uint count)
        :mCount(count)
    {
        D3D11_BUFFER_DESC id = {};
        id.Usage = D3D11_USAGE_DEFAULT;
        id.ByteWidth = sizeof(Uint) * count;
        id.BindFlags = D3D11_BIND_INDEX_BUFFER;
        id.CPUAccessFlags = 0;
        id.MiscFlags = 0;
        id.StructureByteStride = sizeof(Uint);

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = indices;
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;

        DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&id, &sd, &mIndexBuffer));
    }

    DX11IndexBuffer::~DX11IndexBuffer()
    {
        mIndexBuffer->Release();
    }

    void DX11IndexBuffer::Bind() const
    {
        DX11Internal::GetDeviceContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    void DX11IndexBuffer::Unbind() const
    {
        DX11Internal::GetDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
    }
}
