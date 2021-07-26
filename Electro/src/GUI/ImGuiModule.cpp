//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ImGuiModule.hpp"
#include "Core/Application.hpp"
#include <FontAwesome.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <backends/imgui_impl_win32.h>

#include "Platform/DX11/DX11Internal.hpp"
#include <backends/imgui_impl_dx11.h>

namespace Electro
{
    void ImGuiModule::Init()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.FontDefault = io.Fonts->AddFontFromFileTTF("Electro/assets/ElectroFonts/Ruda/Ruda-Bold.ttf", 13.0f);
        io.IniFilename = "ElectroLayout.ini";

        // Add the icons
        ImFontConfig config;
        config.MergeMode = true;
        static const ImWchar iconRanges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
        io.Fonts->AddFontFromFileTTF("Electro/assets/ElectroFonts/fontawesome-webfont.ttf", 13.0f, &config, iconRanges);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        SetDarkThemeColors(); // Electro's theme

        //Init Dear ImGui
        Application& app = Application::Get();
        ImGui_ImplWin32_Init((HWND)app.GetWindow().GetNativeWindow());
        ImGui_ImplDX11_Init(DX11Internal::GetDevice(), DX11Internal::GetDeviceContext());
    }
    void ImGuiModule::OnEvent(Event& e)
    {
        if (mBlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }
    void ImGuiModule::Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiModule::Begin()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void ImGuiModule::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

        // Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImGuiModule::SetDarkThemeColors()
    {
        constexpr auto colorFromBytes = [](const uint8_t r, const uint8_t g, const uint8_t b)
        {
            return ImVec4(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, 1.0f);
        };

        auto& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.TabRounding = 3.5f;
        style.FrameRounding = 3.5f;
        //style.FrameBorderSize = 1.0f;
        style.PopupRounding = 3.5f;
        style.ScrollbarRounding = 3.5f;
        style.GrabRounding = 3.5f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.DisplaySafeAreaPadding = ImVec2(0, 0);

        // Headers
        colors[ImGuiCol_Header] = colorFromBytes(62, 62, 62);
        colors[ImGuiCol_HeaderHovered] = colorFromBytes(56, 56, 56);

        // Checbox
        colors[ImGuiCol_CheckMark] = colorFromBytes(255, 255, 255);

        // Buttons
        colors[ImGuiCol_Button] = colorFromBytes(25, 25, 25);
        colors[ImGuiCol_ButtonHovered] = colorFromBytes(110, 110, 110);
        colors[ImGuiCol_ButtonActive] = colorFromBytes(120, 120, 120);

        // Frame
        colors[ImGuiCol_FrameBg] = colorFromBytes(25, 25, 25);
        colors[ImGuiCol_FrameBgHovered] = colorFromBytes(88, 88, 88);
        colors[ImGuiCol_FrameBgActive] = colorFromBytes(110, 110, 110);

        // Tabs
        colors[ImGuiCol_Tab] = colorFromBytes(56, 56, 56);
        colors[ImGuiCol_TabHovered] = colorFromBytes(56, 56, 56);
        colors[ImGuiCol_TabActive] = colorFromBytes(90, 90, 90);
        colors[ImGuiCol_TabUnfocused] = colorFromBytes(40, 40, 40);
        colors[ImGuiCol_TabUnfocusedActive] = colorFromBytes(88, 88, 88);

        // Title
        colors[ImGuiCol_TitleBg] = colorFromBytes(40, 40, 40);
        colors[ImGuiCol_TitleBgActive] = colorFromBytes(40, 40, 40);

        // Others
        colors[ImGuiCol_WindowBg] = colorFromBytes(45, 45, 45);
        colors[ImGuiCol_PopupBg] = colorFromBytes(45, 45, 45);
        colors[ImGuiCol_DockingPreview] = colorFromBytes(26, 26, 26);

        //colors[ImGuiCol_Separator] = colorFromBytes(10, 200, 10);
        //colors[ImGuiCol_Border] = colorFromBytes(10, 200, 10);
    }
}
