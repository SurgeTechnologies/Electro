//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Window.hpp"
#include "Core/Module.hpp"
#include "Core/ModuleManager.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "Renderer/Renderer.hpp"
#include "GUI/ImGuiModule.hpp"
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
    };

    class Application
    {
    public:
        Application() = default;
        virtual ~Application();
        void Init(const ApplicationProps& applicationProps);
        void Run();
        void OnEvent(Event& e);

        void PushModule(Module* module);
        void PushOverlay(Module* module);
        ImGuiModule* GetImGuiModule() { return mImGuiModule; }

        Window& GetWindow() { return *mWindow; }
        const String& GetCSharpDLLPath() const { return mApplicationProps.ScriptEngineAssemblyPath; }
        const String GetBuildConfig() const;
        void Close();

        static Application& Get() { return *sInstance; }
    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
    private:
        LARGE_INTEGER mStartTime;
        Scope<Window> mWindow;
        ImGuiModule* mImGuiModule;
        bool mRunning = true;
        bool mMinimized = false;
        ModuleManager mModuleManager;
        ApplicationProps mApplicationProps;
        float mLastFrameTime = 0.0f;
    private:
        static Application* sInstance;
    };

    // To be defined in CLIENT
    Application* CreateApplication();
}
