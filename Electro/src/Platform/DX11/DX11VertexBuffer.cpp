//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11VertexBuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    // Dynamic Vertex Buffer
    DX11VertexBuffer::DX11VertexBuffer(Uint size)
    {
        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage = D3D11_USAGE_DYNAMIC;
        vbd.ByteWidth = size;
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vbd.MiscFlags = 0;
        vbd.StructureByteStride = 0;

        DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&vbd, nullptr, &mVertexBuffer)); //Create empty vertex buffer
    }

    DX11VertexBuffer::DX11VertexBuffer(void* vertices, Uint size)
    {
        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = size;
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;
        vbd.MiscFlags = 0;
        vbd.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = vertices;
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;

        DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&vbd, &sd, &mVertexBuffer));
    }

    DX11VertexBuffer::~DX11VertexBuffer()
    {
        mVertexBuffer->Release();
    }

    void DX11VertexBuffer::Bind(Uint stride) const
    {
        Uint offset = 0;
        DX11Internal::GetDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    }

    void DX11VertexBuffer::Unbind() const
    {
        DX11Internal::GetDeviceContext()->IASetVertexBuffers(0, 1, nullptr, 0, 0);
    }

    void DX11VertexBuffer::SetData(const void* data, Uint size)
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE ms = {};
        deviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, data, size);
        deviceContext->Unmap(mVertexBuffer, 0);
    }
}