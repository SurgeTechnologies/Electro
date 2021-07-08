//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PostProcessingPipeline.hpp"
#include "Bloom.hpp"

namespace Electro
{
    void PostProcessingPipeline::Init(const Ref<Renderbuffer>& targetRenderBuffer)
    {
        mStack[BLOOM_METHOD_KEY] = new Bloom();

        for (auto& stackObject : mStack)
            stackObject.second->Init(targetRenderBuffer);
    }

    void PostProcessingPipeline::Shutdown()
    {
        delete mStack.at(BLOOM_METHOD_KEY);
        mStack.clear();
    }

    void PostProcessingPipeline::ProcessAll()
    {
        for (auto& stackObject : mStack)
            stackObject.second->Process();
    }
}