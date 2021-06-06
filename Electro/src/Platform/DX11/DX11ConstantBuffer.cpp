//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11ConstantBuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    D3D11_USAGE ElectroUsageToDX11Usage(DataUsage usage)
    {
        switch (usage)
        {
            case DataUsage::DEFAULT: return D3D11_USAGE_DEFAULT;
            case DataUsage::DYNAMIC: return D3D11_USAGE_DYNAMIC;
        }
        return (D3D11_USAGE)-1;
    }

    DX11ConstantBuffer::DX11ConstantBuffer(Uint size, Uint bindSlot, DataUsage usage)
        : mBuffer(nullptr), mSize(size), mBindSlot(bindSlot), mDataUsage(usage), mData(nullptr)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = ElectroUsageToDX11Usage(usage);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = (ElectroUsageToDX11Usage(usage) == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &mBuffer)); //Create an empty CBuffer
    }

    const void DX11ConstantBuffer::VSBind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->VSSetConstantBuffers(mBindSlot, 1, &mBuffer);
    }

    const void DX11ConstantBuffer::PSBind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->PSSetConstantBuffers(mBindSlot, 1, &mBuffer);
    }

    const void DX11ConstantBuffer::CSBind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->CSSetConstantBuffers(mBindSlot, 1, &mBuffer);
    }

    const void DX11ConstantBuffer::SetStaticData(void* data) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->UpdateSubresource(mBuffer, 0, 0, data, 0, 0);
    }

    const void DX11ConstantBuffer::SetDynamicData(void* data) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE ms = {};
        deviceContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, data, mSize);
        deviceContext->Unmap(mBuffer, 0);
    }

    DX11ConstantBuffer::~DX11ConstantBuffer()
    {
        mBuffer->Release();
    }
}
