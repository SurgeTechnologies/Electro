//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroApplication.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroRenderer2D.hpp"

namespace Electro
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

    Application* Application::sInstance = nullptr;
    Application::Application(const char* name)
    {
        E_ASSERT(!sInstance, "Application already exists!");

        SplashWindowProps props;
        props.Name = "Electro";
        props.ImagePath = "Resources/Icon/electro.bmp";
        props.Width = 400;
        props.Height = 400;
        auto splashWindow = SplashWindow::Create(props);

        QueryPerformanceCounter(&mStartTime);
        sInstance = this;

        mWindow = Window::Create(WindowProps("Electro", 1280, 720));
        mWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));
        Renderer::Init();
        Renderer2D::Init();

        mImGuiLayer = new ImGuiLayer();
        PushOverlay(mImGuiLayer);

        mWindow->Present();
        splashWindow->Destroy();
    }

    Application::~Application()
    {
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

    void Application::Close()
    {
        mRunning = false;
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
        mRunning = false;
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