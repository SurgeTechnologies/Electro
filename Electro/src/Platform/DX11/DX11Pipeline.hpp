//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroPipeline.hpp"
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
        virtual PipelineSpecification& GetSpecification() override { return mSpec; }
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) override { mPrimitiveTopology = topology; }
    private:
        ID3D11InputLayout* mInputLayout = nullptr;
        PipelineSpecification mSpec;
        PrimitiveTopology mPrimitiveTopology = PrimitiveTopology::TRIANGLELIST;
    };
}
