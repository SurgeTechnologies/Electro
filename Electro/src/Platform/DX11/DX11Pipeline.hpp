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
        DX11Pipeline(const PipelineSpecification& spec);
        virtual ~DX11Pipeline();
        virtual void Bind() const override;
        virtual void BindSpecificationObjects() const override;
        virtual void Unbind() const override;
        virtual const PipelineSpecification& GetSpecification() const override { return mSpec; }
    private:
        ID3D11InputLayout* mInputLayout = nullptr;
        PipelineSpecification mSpec;
    };
}
