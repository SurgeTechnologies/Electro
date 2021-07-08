//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Renderbuffer.hpp"

#define BLOOM_METHOD_KEY "Bloom"

namespace Electro
{
    class IPostProcessMethod
    {
    public:
        virtual ~IPostProcessMethod() = default;

        virtual void Init(const Ref<Renderbuffer>& target) = 0;
        virtual void Process() = 0;
        virtual const Ref<Renderbuffer>& GetOutputRenderBuffer() const = 0;
    };

    // Mapped as                                 ->Name  | Method
    using PostProcessingStack = std::unordered_map<String, IPostProcessMethod*>;

    class PostProcessingPipeline
    {
    public:
        PostProcessingPipeline() = default;
        ~PostProcessingPipeline() = default;

        void Init(const Ref<Renderbuffer>& targetRenderBuffer);
        void Shutdown();
        void ProcessAll();
        const PostProcessingStack& GetStack() const { return mStack; }

        template<typename T>
        T* GetMethodByKey(const String& key)
        {
            return static_cast<T*>(mStack.at(key));
        }

    private:
        PostProcessingStack mStack;
    };
}