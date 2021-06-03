//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Window.hpp"
#include "Core/SplashWindow.hpp"
#include "Core/Module.hpp"
#include "Core/ModuleManager.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "GUI/ImGuiModule.hpp"
#include <Windows.h>

//Comment this line to toggle turn off the Electro editor - everything is rendered to backbuffer then
#define SHOW_EDITOR

namespace Electro
{
    class Application
    {
    public:
        Application(const char* name = "ElectroApplication");
        virtual ~Application();
        void Run();
        void OnEvent(Event& e);

        void PushModule(Module* module);
        void PushOverlay(Module* module);
        ImGuiModule* GetImGuiModule() { return mImGuiModule; }

        Window& GetWindow() { return *mWindow; }
        const String& GetCSharpDLLPath() const { return mCSAppAssemblyPath; }
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
        float mLastFrameTime = 0.0f;
        String mCSAppAssemblyPath;
    private:
        static Application* sInstance;
    };
    // To be defined in CLIENT
    Application* CreateApplication();
}
