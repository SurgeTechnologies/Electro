//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Renderbuffer.hpp"

#define BLOOM_METHOD_KEY 0

namespace Electro
{
    class IPostProcessingEffect
    {
    public:
        virtual ~IPostProcessingEffect() = default;
        virtual void Init(const Ref<Renderbuffer>& target) = 0;
        virtual void Process() = 0;

        virtual bool IsEnabled() const { return mEnabled; };
        virtual void SetEnabled(bool enabled) const { mEnabled = enabled; };

        virtual const Ref<Renderbuffer>& GetOutputRenderBuffer() const = 0;
    protected:
        mutable bool mEnabled;
    };

    class PostProcessingPipeline
    {
    public:
        PostProcessingPipeline() = default;
        ~PostProcessingPipeline() = default;

        // Initialize the post processing pipeline
        void Init(const Ref<Renderbuffer>& targetRenderBuffer);

        // Shutdown the post processing pipeline, removes all allocated memory
        void Shutdown();

        // Process All the Post Processing Effects in the stack
        void ProcessAll();

        template<typename T>
        T* GetEffectByKey(Uint key)
        {
            return dynamic_cast<T*>(mStack[key]);
        }
    private:
        Vector<IPostProcessingEffect*> mStack; // Post processing stack
    };
}