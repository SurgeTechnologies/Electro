//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroWindow.hpp"
#include "Core/ElectroSplashWindow.hpp"
#include "Core/ElectroLayer.hpp"
#include "Core/ElectroLayerStack.hpp"
#include "Core/Events/ElectroApplicationEvent.hpp"
#include "GUI/ElectroImGuiLayer.hpp"

namespace Electro
{
    class Application
    {
    public:
        Application(const char* name = "ElectroApplication");
        virtual ~Application();
        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        Window& GetWindow() { return *mWindow; }
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
        LayerStack mLayerStack;
        float mLastFrameTime = 0.0f;
    private:
        static Application* sInstance;
    };
    // To be defined in CLIENT
    Application* CreateApplication();
}
