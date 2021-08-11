//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IRenderPass.hpp"

namespace Electro
{
    struct RendererData;
    class RenderPassManager
    {
    public:
        void Init(RendererData* rendererData);
        void Shutdown();

        void AddRenderPass(IRenderPass* renderPass)
        {
            renderPass->Init(mRendererData);
            mRenderPasses.push_back(renderPass);
        }

        template <typename T>
        T* GetRenderPass()
        {
            static_assert(std::is_base_of<IRenderPass, T>::value, "Class must derive from IRenderPass");

            for (IRenderPass* pass : mRenderPasses)
            {
                T* result = dynamic_cast<T*>(pass);
                if (result)
                {
                    return result;
                }
            }
            E_INTERNAL_ASSERT("Undefined Render pass!");
            return nullptr;
        }

        template <typename T>
        typename T::InternalData* GetRenderPassData()
        {
            static_assert(std::is_base_of<IRenderPass, T>::value, "Class must derive from IRenderPass");

            for (IRenderPass* pass : mRenderPasses)
            {
                T* result = dynamic_cast<T*>(pass);
                if (result)
                {
                    return static_cast<typename T::InternalData*>(result->GetInternalDataBlock());
                }
            }
            E_INTERNAL_ASSERT("Undefined Render pass!");
            return nullptr;
        }
    private:
        Vector<IRenderPass*> mRenderPasses;
        RendererData* mRendererData = nullptr;
    };
}