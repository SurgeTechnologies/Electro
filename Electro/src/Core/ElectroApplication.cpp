//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroApplication.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"

namespace Electro
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application* Application::sInstance = nullptr;
    Application::Application(const char* name)
    {
        E_ASSERT(!sInstance, "Application already exists!");

        SplashWindowProps props;
        props.Name = "Electro";
        props.ImagePath = "Resources/Branding/ElectroEngine.bmp";
        props.Width = 332;
        props.Height = 439;
        auto splashWindow = SplashWindow::Create(props);

        QueryPerformanceCounter(&mStartTime);
        sInstance = this;

        mWindow = Window::Create(WindowProps("Electro", 1280, 720));
        mWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));
        mCSAppAssemblyPath = "ExampleApp/bin/Debug/ExampleApp.dll";

        Renderer::Init();
        Renderer2D::Init();
        ScriptEngine::Init(mCSAppAssemblyPath.c_str());
        PhysicsEngine::Init();

        mImGuiLayer = new ImGuiLayer();
        PushOverlay(mImGuiLayer);

        mWindow->Present();
        splashWindow->Destroy();
    }

    Application::~Application()
    {
        Renderer::Shutdown();
        Renderer2D::Shutdown();
        ScriptEngine::Shutdown();
        PhysicsEngine::ShutDown();
    }

    void Application::PushLayer(Layer* layer)
    {
        mLayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        mLayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    String Application::GetBuildConfig()
    {
#ifdef E_DEBUG
        return String("Debug");
#elif defined E_RELEASE
        return String("Release");
#endif
    }

    void Application::Close()
    {
        if (OS::AMessageBox("Are you sure you what to exit Electro?", "Save any unsaved changes, they can not be recovered if you exit the Engine!", DialogType::Yes__No, IconType::Warning, DefaultButton::No))
            mRunning = false;
        return;
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

        for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
        {
            (*--it)->OnEvent(e);
            if (e.Handled)
                break;
        }
    }

    //Engines main RUN loop
    void Application::Run()
    {
        while (mRunning)
        {
            LARGE_INTEGER currentTime, frequency;
            QueryPerformanceCounter(&currentTime);
            QueryPerformanceFrequency(&frequency);

            float time = static_cast<float>((currentTime.QuadPart - mStartTime.QuadPart) / (double)frequency.QuadPart);
            Timestep timestep = time - mLastFrameTime;
            mLastFrameTime = time;

            if (!mMinimized)
            {
                {
                    for (Layer* layer : mLayerStack)
                        layer->OnUpdate(timestep);
                }

                {
                    mImGuiLayer->Begin();
                    for (Layer* layer : mLayerStack)
                        layer->OnImGuiRender();
                    mImGuiLayer->End();
                }
            }

            mWindow->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        Close();
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetHeight() == 0 || e.GetWidth() == 0)
        {
            mMinimized = true;
            return false;
        }
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        mMinimized = false;
        return false;
    }
}