//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Renderer.hpp"
#include "IPanel.hpp"

namespace Electro
{
    class ProfilerPanel : public IPanel
    {
    public:
        ProfilerPanel() = default;
        ~ProfilerPanel() = default;

        virtual void OnInit(void* data) override;
        virtual void OnImGuiRender(bool* show) override;

    private:
        float mTime = 0.0f;
        bool mVSync;
        float mFPSValues[50];
        Vector<float> mFrameTimes;
        RenderAPICapabilities mRenderCaps;
        RendererData* mRendererData;
    };
}
