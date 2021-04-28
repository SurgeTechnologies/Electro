//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroProfilerPanel.hpp"
#include "Core/ElectroApplication.hpp"
#include "EDevice/EDevice.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroUIMacros.hpp"
#include <imgui.h>

namespace Electro
{
    void ProfilerPanel::OnImGuiRender(bool* show)
    {
        float avg = 0.0f;
        const uint32_t size = static_cast<Uint>(mFrameTimes.size());
        if (size >= 50)
            mFrameTimes.erase(mFrameTimes.begin());

        mFrameTimes.push_back(ImGui::GetIO().Framerate);
        for (uint32_t i = 0; i < size; i++)
        {
            mFPSValues[i] = mFrameTimes[i];
            avg += mFrameTimes[i];
        }
        avg /= size;

        ImGui::Begin(PROFILER_TITLE, show);
        auto& caps = RendererAPI::GetCapabilities();
        ImGui::Text("Vendor: %s", caps.Vendor.c_str());
        ImGui::Text("Renderer: %s", caps.Renderer.c_str());
        ImGui::Text("Version: %s", caps.Version.c_str());
        ImGui::Text("Max Texture Slots: %d", caps.MaxTextureUnits);
        ImGui::Text("Max Samples: %d", caps.MaxSamples);
        ImGui::Text("Max Anisotropy: %f", caps.MaxAnisotropy);

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
        ImGui::Separator();
        UI::TextCentered("Renderer");
        ImGui::Text("DrawCalls: %i", Renderer::GetTotalDrawCallsCount());
        ImGui::Separator();
        auto& stats2D = Renderer2D::GetStats();
        UI::TextCentered("Renderer2D");
        ImGui::Text("Draw Calls: %i", stats2D.DrawCalls);
        ImGui::Text("Quad Count: %d", stats2D.QuadCount);
        ImGui::Text("Vertices: %d", stats2D.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats2D.GetTotalIndexCount());

        EDeviceStatus& deviceStatus = EDevice::GetEDeviceStatus();
        ImGui::Separator();
        UI::TextCentered("EDevice");
        ImGui::Text("TotalVertexBuffers: %i", deviceStatus.TotalVertexBuffers);
        ImGui::Text("TotalIndexBuffers: %i", deviceStatus.TotalIndexBuffers);
        ImGui::Text("TotalShaders: %i", deviceStatus.TotalShaders);
        ImGui::Text("TotalConstantBuffers: %i", deviceStatus.TotalConstantBuffers);
        ImGui::Text("TotalPipelines: %i", deviceStatus.TotalPipelines);
        ImGui::Text("TotalTexture2Ds: %i", deviceStatus.TotalTexture2Ds);
        ImGui::Text("TotalCubemaps: %i", deviceStatus.TotalCubemaps);
        ImGui::End();
    }
}
