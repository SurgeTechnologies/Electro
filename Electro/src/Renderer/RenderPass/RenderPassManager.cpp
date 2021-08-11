//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "RenderPassManager.hpp"

namespace Electro
{
    void RenderPassManager::Init(RendererData* rendererData)
    {
        mRendererData = rendererData;
    }

    void RenderPassManager::Shutdown()
    {
        for (IRenderPass* pass : mRenderPasses)
        {
            pass->Shutdown();
            delete pass;
        }

        mRenderPasses.clear();
    }
}