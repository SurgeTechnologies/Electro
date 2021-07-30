//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Window.hpp"
#include "Core/Layer.hpp"
#include "Core/LayerManager.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "Renderer/Renderer.hpp"
#include "GUI/ImGuiLayer.hpp"
#include <Windows.h>

namespace Electro
{
    struct ApplicationProps
    {
        ApplicationProps() = default;
        bool VSync = true;
        bool EnableImGui = true;
        RendererBackend RendererBackend;
        WindowProps WindowData;
        String ScriptEngineAssemblyPath;
        bool IsRuntime = false;
    };

    class Application
    {
    public:
        Application() = default;
        virtual ~Application();
        void Init(const ApplicationProps& applicationProps);
        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        ImGuiLayer* GetImGuiLayer() { return mImGuiLayer; }

        Window& GetWindow() { return *mWindow; }
        const String& GetCSharpDLLPath() const { return mApplicationProps.ScriptEngineAssemblyPath; }
        const String GetBuildConfig() const;
        bool IsRuntime() { return mApplicationProps.IsRuntime; }
        void Close();

        static Application& Get() { return *sInstance; }
    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
    private:
        LARGE_INTEGER mStartTime;
        Scope<Window> mWindow;
        ImGuiLayer* mImGuiLayer;
        bool mRunning = true;
        bool mMinimized = false;
        LayerManager mLayerManager;
        ApplicationProps mApplicationProps;
        float mLastFrameTime = 0.0f;
    private:
        static Application* sInstance;
    };

    // To be defined in CLIENT
    Application* CreateApplication();
}
