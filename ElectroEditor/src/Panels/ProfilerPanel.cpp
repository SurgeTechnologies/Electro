//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ProfilerPanel.hpp"
#include "Core/Application.hpp"
#include "Renderer/RendererAPI.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"
#include <imgui.h>

namespace Electro
{
    void ProfilerPanel::Init(void* data)
    {
        mRenderCaps = RendererAPI::GetCapabilities();
    }

    void ProfilerPanel::OnImGuiRender(bool* show)
    {
        float avg = 0.0f;
        const Uint size = static_cast<Uint>(mFrameTimes.size());
        if (size >= 50)
            mFrameTimes.erase(mFrameTimes.begin());

        mFrameTimes.push_back(ImGui::GetIO().Framerate);
        for (Uint i = 0; i < size; i++)
        {
            mFPSValues[i] = mFrameTimes[i];
            avg += mFrameTimes[i];
        }
        avg /= size;

        ImGui::Begin(PROFILER_TITLE, show);
        ImGui::Text("Vendor: %s", mRenderCaps.Vendor.c_str());
        ImGui::Text("Renderer: %s", mRenderCaps.Renderer.c_str());
        ImGui::Text("Version: %s", mRenderCaps.Version.c_str());
        ImGui::Text("Max Texture Slots: %d", mRenderCaps.MaxTextureUnits);
        ImGui::Text("Max Samples: %d", mRenderCaps.MaxSamples);
        ImGui::Text("Max Anisotropy: %f", mRenderCaps.MaxAnisotropy);

        const float fps = (1.0f / avg) * 1000.0f;
        ImGui::Text("Frame time (ms): %f", fps);

        ImGui::Text("FPS: %f", avg);
        ImGui::SameLine();
        ImGui::Columns(2);
        ImGui::NextColumn();
        ImGui::SetColumnWidth(0, 130.0f);
        ImGui::PlotHistogram("##FPS", mFPSValues, size);
        ImGui::Columns(1);
        mVSync = Application::Get().GetWindow().IsVSync();
        if (UI::Checkbox("VSync Enabled", &mVSync, 130.0f))
            Application::Get().GetWindow().SetVSync(mVSync);
        ImGui::End();
    }
}
