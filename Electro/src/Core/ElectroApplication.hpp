//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroWindow.hpp"
#include "Core/ElectroSplashWindow.hpp"
#include "Core/ElectroModule.hpp"
#include "Core/ElectroModuleManager.hpp"
#include "Core/Events/ElectroApplicationEvent.hpp"
#include "GUI/ElectroImGuiModule.hpp"

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
        String& GetCSharpDLLPath() { return mCSAppAssemblyPath; }
        String GetBuildConfig();
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
