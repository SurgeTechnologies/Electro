//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11ConstantBuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    D3D11_USAGE SpikeUsageToDX11Usage(DataUsage usage)
    {
        switch (usage)
        {
            case DataUsage::DEFAULT: return D3D11_USAGE_DEFAULT;
            case DataUsage::DYNAMIC: return D3D11_USAGE_DYNAMIC;
        }
        return (D3D11_USAGE)-1;
    }

    DX11ConstantBuffer::DX11ConstantBuffer(const Ref<Shader>& shader, const String& name, void* data, const Uint size, const Uint bindSlot, ShaderDomain shaderDomain, DataUsage usage)
        :mBindSlot(bindSlot), mSize(size), m_ShaderDomain(shaderDomain), mDataUsage(usage)
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = ((size / 16) + 1) * 16; //Align by 16 bytes
        bufferDesc.Usage = SpikeUsageToDX11Usage(usage);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = (SpikeUsageToDX11Usage(usage) == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        if (data != nullptr)
        {
            D3D11_SUBRESOURCE_DATA sd = {};
            sd.pSysMem = data;
            sd.SysMemPitch = 0;
            sd.SysMemSlicePitch = 0;
            DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&bufferDesc, &sd, &mBuffer));
        }
        else
            DX_CALL(DX11Internal::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &mBuffer)); //Create an empty CBuffer
    }

    void DX11ConstantBuffer::Bind()
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        switch (m_ShaderDomain)
        {
            case ShaderDomain::NONE:   break;
            case ShaderDomain::VERTEX: deviceContext->VSSetConstantBuffers(mBindSlot, 1, &mBuffer); break;
            case ShaderDomain::PIXEL:  deviceContext->PSSetConstantBuffers(mBindSlot, 1, &mBuffer); break;
        }
    }

    void DX11ConstantBuffer::SetData(void* data)
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        switch (mDataUsage)
        {
            case DataUsage::DEFAULT:
                deviceContext->UpdateSubresource(mBuffer, 0, 0, data, 0, 0); Bind(); break;
            case DataUsage::DYNAMIC:
                D3D11_MAPPED_SUBRESOURCE ms = {};
                deviceContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
                memcpy(ms.pData, data, mSize);
                deviceContext->Unmap(mBuffer, 0); Bind(); break;
        }
    }

    DX11ConstantBuffer::~DX11ConstantBuffer()
    {
        mBuffer->Release();
    }
}
