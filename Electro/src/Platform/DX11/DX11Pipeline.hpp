//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Pipeline.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11Pipeline : public Pipeline
    {
    public:
        DX11Pipeline() = default;
        virtual ~DX11Pipeline();
        virtual void Bind() const override;
        virtual void GenerateInputLayout(const Ref<Shader>& shader) override;
        virtual const Uint GetStride() const override { return mStride; }
    private:
        ID3D11InputLayout* mInputLayout = nullptr;
        Uint mStride = 0;
    };
}
