//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IRenderPass.hpp"

namespace Electro
{
    class GeometryPass : public IRenderPass
    {
    public:
        virtual void Init(RendererData* rendererData) override;
        virtual void Update() override;
        virtual void Shutdown() override;
    protected:
        virtual void* GetInternalDataBlock() override { E_INTERNAL_ASSERT("Geometry Pass doesn't have any internal data!") return nullptr; }
    public:
        struct InternalData
        {
            // Nothing, it's empty
        };
    private:
        friend class RenderPassManager;
    };
}