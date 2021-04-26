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
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Electro/assets/fonts/Ruda/Ruda-SemiBold.ttf", 13.0f);

        // Add the icons
        ImFontConfig config;
        config.MergeMode = true;
        static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
        io.Fonts->AddFontFromFileTTF("Electro/assets/fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);
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
        constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
        {
            return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
        };

        auto& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.TabRounding = 3.5f;
        style.FrameRounding = 3.5f;
        style.PopupRounding = 3.5f;
        style.ScrollbarRounding = 3.5f;
        style.GrabRounding = 3.5f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.DisplaySafeAreaPadding = ImVec2(0, 0);

        // Headers
        colors[ImGuiCol_Header] = ColorFromBytes(62, 62, 62);
        colors[ImGuiCol_HeaderHovered] = ColorFromBytes(56, 56, 56);

        // Checbox
        colors[ImGuiCol_CheckMark] = ColorFromBytes(255, 255, 255);

        // Buttons
        colors[ImGuiCol_Button] = ColorFromBytes(42, 42, 42);
        colors[ImGuiCol_ButtonHovered] = ColorFromBytes(110, 110, 110);
        colors[ImGuiCol_ButtonActive] = ColorFromBytes(120, 120, 120);

        // Frame
        colors[ImGuiCol_FrameBg] = ColorFromBytes(25, 25, 25);
        colors[ImGuiCol_FrameBgHovered] = ColorFromBytes(88, 88, 88);
        colors[ImGuiCol_FrameBgActive] = ColorFromBytes(110, 110, 110);

        // Tabs
        colors[ImGuiCol_Tab] = ColorFromBytes(56, 56, 56);
        colors[ImGuiCol_TabHovered] = ColorFromBytes(56, 56, 56);
        colors[ImGuiCol_TabActive] = ColorFromBytes(90, 90, 90);
        colors[ImGuiCol_TabUnfocused] = ColorFromBytes(40, 40, 40);
        colors[ImGuiCol_TabUnfocusedActive] = ColorFromBytes(88, 88, 88);

        // Title
        colors[ImGuiCol_TitleBg] = ColorFromBytes(40, 40, 40);
        colors[ImGuiCol_TitleBgActive] = ColorFromBytes(40, 40, 40);

        //Others
        colors[ImGuiCol_WindowBg] = ColorFromBytes(45, 45, 45);
        colors[ImGuiCol_DockingPreview] = ColorFromBytes(26, 26, 26);
    }
}
