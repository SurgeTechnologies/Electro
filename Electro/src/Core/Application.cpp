//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Application.hpp"
#include "Core/System/OS.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "Asset/AssetManager.hpp"

namespace Electro
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
    Application* Application::sInstance = nullptr;

    void Application::Init(const ApplicationProps& applicationProps)
    {
        E_ASSERT(!sInstance, "Application already exists!");
        sInstance = this;
        QueryPerformanceCounter(&mStartTime);
        mApplicationProps = applicationProps;
        mWindow = OS::CreateAppWindow(mApplicationProps.WindowData);
        mWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

        Renderer::SetBackend(applicationProps.RendererBackend);

        Renderer::Init();
        Renderer2D::Init();
        ScriptEngine::Init(mApplicationProps.ScriptEngineAssemblyPath.c_str());
        PhysicsEngine::Init();

        if (applicationProps.EnableImGui)
        {
            mImGuiModule = new ImGuiModule();
            PushOverlay(mImGuiModule);
        }

        mWindow->SetVSync(applicationProps.VSync);
        mWindow->Present();
    }

    Application::~Application()
    {
        Renderer::Shutdown();
        Renderer2D::Shutdown();
        ScriptEngine::Shutdown();
        PhysicsEngine::ShutDown();
        AssetManager::Shutdown();
    }

    void Application::PushModule(Module* module)
    {
        mModuleManager.PushModule(module);
        module->Init();
    }

    void Application::PushOverlay(Module* module)
    {
        mModuleManager.PushOverlay(module);
        module->Init();
    }

    const String Application::GetBuildConfig() const
    {
#ifdef E_DEBUG
        return String("Debug x64");
#elif defined E_RELEASE
        return String("Release x64");
#endif
    }

    void Application::Close()
    {
        mRunning = false;
        return;
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

        for (auto it = mModuleManager.end(); it != mModuleManager.begin();)
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
                for (Module* m : mModuleManager)
                    m->OnUpdate(timestep);

                if (mApplicationProps.EnableImGui)
                {
                    mImGuiModule->Begin();
                    for (Module* m : mModuleManager)
                        m->OnImGuiRender();
                    mImGuiModule->End();
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
