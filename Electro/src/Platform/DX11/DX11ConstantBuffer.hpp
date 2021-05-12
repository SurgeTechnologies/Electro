//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ConstantBuffer.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11ConstantBuffer : public ConstantBuffer
    {
    public:
        DX11ConstantBuffer(Uint size, Uint bindSlot, DataUsage usage = DataUsage::DYNAMIC);
        ~DX11ConstantBuffer();
        virtual const void VSBind() const override;
        virtual const void PSBind() const override;
        virtual const void CSBind() const override;
        virtual const void* GetData() const override { return mData; }
        virtual const void SetStaticData(void* data) const override;
        virtual const void SetDynamicData(void* data) const override;
        virtual Uint GetSize() override { return mSize; }

        virtual RendererID GetNativeBuffer() override { return (RendererID)mBuffer; }
        virtual DataUsage GetDataUsage() override { return mDataUsage; }
    private:
        ID3D11Buffer* mBuffer;
        Uint mSize;
        Uint mBindSlot;
        DataUsage mDataUsage;
        void* mData;
    };
}
