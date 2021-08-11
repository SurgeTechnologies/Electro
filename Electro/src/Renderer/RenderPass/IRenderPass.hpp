//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Renderbuffer.hpp"

namespace Electro
{
    struct RendererData;
    class IRenderPass
    {
    public:
        IRenderPass() = default;
        virtual ~IRenderPass() = default;

        virtual void Init(RendererData* rendererData) = 0;
        virtual void Update() = 0;
        virtual void Shutdown() = 0;
    protected:
        virtual void* GetInternalDataBlock() = 0;
    protected:
        RendererData* mRendererData;
    };
}
