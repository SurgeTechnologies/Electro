//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroImGuiLayer.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "Core/ElectroApplication.hpp"

#include <FontAwesome.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <backends/imgui_impl_win32.h>

#ifdef RENDERER_API_DX11
    #include "Platform/DX11/DX11Internal.hpp"
    #include <backends/imgui_impl_dx11.h>
#else
    #error No RendererAPI detected
#endif

namespace Electro
{
    void ImGuiLayer::OnAttach()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Electro/assets/fonts/Ruda/Ruda-Regular.ttf", 15.0f);

        // Add the icons
        ImFontConfig config;
        config.MergeMode = true;
        static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
        io.Fonts->AddFontFromFileTTF("Electro/assets/fonts/fontawesome-webfont.ttf", 14.0f, &config, icon_ranges);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        SetDarkThemeColors(); // Electro's theme

        //Init Dear ImGui
        Application& app = Application::Get();
    #ifdef RENDERER_API_DX11
        ImGui_ImplWin32_Init((HWND)app.GetWindow().GetNativeWindow());
        ImGui_ImplDX11_Init(DX11Internal::GetDevice(), DX11Internal::GetDeviceContext());
    #else
        #error No RendererAPI detected
    #endif
    }
    void ImGuiLayer::OnEvent(Event& e)
    {
        if (mBlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }
    void ImGuiLayer::OnDetach()
    {
    #ifdef RENDERER_API_DX11
        ImGui_ImplDX11_Shutdown();
    #else
        #error No RendererAPI detected
    #endif

        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
    #ifdef RENDERER_API_DX11
        ImGui_ImplDX11_NewFrame();
    #else
        #error No RendererAPI detected
    #endif
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
    #ifdef RENDERER_API_DX11
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    #else
        #error No RendererAPI detected
    #endif

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImGuiLayer::SetDarkThemeColors()
    {
        auto& style = ImGui::GetStyle();
        auto& colors = ImGui::GetStyle().Colors;

        style.TabRounding       = 3.5f;
        style.FrameRounding     = 3.5f;
        style.PopupRounding     = 3.5f;
        style.ScrollbarRounding = 3.5f;
        style.GrabRounding      = 3.5f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
        ImVec4 standardColor = ImVec4(0.0980f, 0.46667f, 0.890196f, 1.0f);

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Checbox
        colors[ImGuiCol_CheckMark] = standardColor;

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = standardColor;
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = standardColor;
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = standardColor;
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    }
}