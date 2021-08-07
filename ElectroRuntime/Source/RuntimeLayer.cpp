//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#define ELECTRO_ENTRYPOINT
#include <Electro.hpp>

namespace Electro
{
    class RuntimeLayer : public Layer
    {
    public:
        virtual void Init() override
        {
            // Create the project and the scene
            mProject = Ref<Project>::Create();
            mActiveScene = Ref<Scene>::Create("Runtime");

            // Make sure renderer renders to the swapchain
            Renderer::RenderToSwapchain();

            // Load the project
            DeserializeProject();

            // Load the first scene
            DeserializeScene();

            // Set render and script-engine context
            Renderer::SetSceneContext(mActiveScene.Raw());
            ScriptEngine::SetSceneContext(mActiveScene);

            const Window& window = Application::Get().GetWindow();
            mActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
            mActiveScene->OnRuntimeStart();
        }

        virtual void OnUpdate(Timestep ts) override
        {
            // Clear the backbuffer
            RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
            RenderCommand::Clear();

            // Simulate Physics
            mActiveScene->OnUpdate(ts);

            // Render
            mActiveScene->OnUpdateRuntime(ts);

            RenderCommand::BindBackbuffer();
        }

        virtual void OnEvent(Event& e) override
        {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)
            {
                const RenderbufferSpecification& spec = RenderCommand::GetBackBuffer()->GetSpecification();
                mActiveScene->OnViewportResize(spec.Width, spec.Height);
                return false;
            });
        }

        virtual void OnImGuiRender() override {}

        virtual void Shutdown() override
        {
            mActiveScene->OnRuntimeStop();
        }
    private:
        void DeserializeScene()
        {
            const String& path = fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), mProject->GetConfig().ScenePaths[0]);

            SceneSerializer ds(mActiveScene);
            if (!ds.Deserialize(path))
                Log::Error("Failed to deserialize scene from {0}", path);
        }

        void DeserializeProject()
        {
            // Finds the .eproj file in the root directory
            String ext(".eproj");
            String eprojFileName;
            for (auto& p : std::filesystem::directory_iterator("."))
            {
                if (p.path().extension() == ext)
                {
                    eprojFileName = p.path().filename().string();
                    Log::Info("Loading: {0}", eprojFileName);
                    break;
                }
            }

            if (ProjectSerializer ps; ps.Deserialize(mProject.Raw(), eprojFileName))
            {
                String executablePath = CurrentAppPath::GetExecutablePath();
                mProject->GetConfig().ProjectDirectory = FileSystem::GetParentPath(executablePath);
                ProjectManager::SetActive(mProject);
            }
            else
                Log::Error("Failed to deserialize project");
        }
    private:
        Ref<Scene> mActiveScene;
        Ref<Project> mProject;
    };

    class ElectroRuntime : public Application
    {
    public:
        ElectroRuntime()
        {
            ApplicationProps appProps;
            appProps.WindowData.Title = "Electro Runtime";
            appProps.WindowData.Width = 1280;
            appProps.WindowData.Height = 720;
            appProps.VSync = false;
            appProps.EnableImGui = false;
            appProps.RendererBackend = RendererBackend::DirectX11;
            appProps.ScriptEngineAssemblyPath = "Electro/Scripts/ExampleApp.dll";
            appProps.IsRuntime = true;
            Init(appProps);
            GetWindow().ShowConsole(false);
            PushLayer(new RuntimeLayer());
        }

        ~ElectroRuntime() = default;
    };

    Application* CreateApplication()
    {
        return new ElectroRuntime();
    }
}
