//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PostProcessingPipeline.hpp"
#include "Bloom.hpp"

namespace Electro
{
    void PostProcessingPipeline::Init(const Ref<Renderbuffer>& targetRenderBuffer)
    {
        mStack.push_back(new Bloom());

        // Initialize all the PostProcessingEffects
        for (IPostProcessingEffect* stackObject : mStack)
            stackObject->Init(targetRenderBuffer);
    }

    void PostProcessingPipeline::Shutdown()
    {
        // Clear all the allocated stack effects
        for (IPostProcessingEffect* stackObject : mStack)
            delete stackObject;

        mStack.clear();
    }

    void PostProcessingPipeline::ProcessAll()
    {
        for (auto& stackObject : mStack)
            stackObject->Process();
    }
}