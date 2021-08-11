//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IRenderPass.hpp"

namespace Electro
{
    class DebugPass : public IRenderPass
    {
    public:
        virtual void Init(RendererData* rendererData) override;
        virtual void Update() override;
        virtual void Shutdown() override;
    protected:
        virtual void* GetInternalDataBlock() override { return &mData; };
    public:
        struct InternalData
        {
            Ref<Shader> SolidColorShader;
            Ref<Shader> OutlineShader;
            Ref<Shader> GridShader;

            Ref<Renderbuffer> OutlineRenderBuffer;

            // Options
            bool ShowGrid = true;
            bool ShowCameraFrustum = true;
            bool ShowAABB = false;
        };
    private:
        InternalData mData;
        friend class RenderPassManager;
    };
}
