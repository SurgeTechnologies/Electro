//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include <Electro.hpp>
#include "EditorModule.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include <fstream>

namespace Electro
{
    EditorModule::EditorModule()
        : mVaultPanel(this)
    {
        Texture2D::Create("Resources/ThirdParty/physx.png");
        Texture2D::Create("Electro/assets/textures/Prototype.png");
        Texture2D::Create("Electro/assets/textures/Folder.png");
        Texture2D::Create("Electro/assets/textures/CSharpIcon.png");
        Texture2D::Create("Electro/assets/textures/ElectroIcon.png");
        Texture2D::Create("Electro/assets/textures/UnknownIcon.png");
        Texture2D::Create("Electro/assets/textures/3DFileIcon.png");
        Texture2D::Create("Electro/assets/textures/ImageIcon.png");
        Texture2D::Create("Electro/assets/textures/Material.png");
        Texture2D::Create("Electro/assets/textures/PhysicsMaterial.png");
    }

    void EditorModule::Init()
    {
        Application::Get().GetWindow().RegisterEditorModule(this);

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.SwapChainTarget = false;
        mFramebuffer = Framebuffer::Create(fbSpec);

        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1024.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle("<Null Project>");
        ScriptEngine::SetSceneContext(mEditorScene);
        Renderer::SetActiveRenderBuffer(mFramebuffer);
        Renderer::SetSceneContext(mEditorScene.Raw());

        mPhysicsSettingsPanel.Init();
        mVaultPanel.Init();
        mSceneHierarchyPanel.Init();
        mMaterialPanel.Init();
    }

    void EditorModule::Shutdown() {}

    void EditorModule::OnScenePlay()
    {
        ScriptEngine::ReloadAssembly(Application::Get().GetCSharpDLLPath());

        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneState = SceneState::Play;
        mRuntimeScene = Ref<Scene>::Create(true);
        mEditorScene->CopySceneTo(mRuntimeScene);
        mRuntimeScene->OnRuntimeStart();
        mSceneHierarchyPanel.SetContext(mRuntimeScene);
        Renderer::SetSceneContext(mRuntimeScene.Raw());
    }

    void EditorModule::OnSceneStop()
    {
        mRuntimeScene->OnRuntimeStop();
        mSceneState = SceneState::Edit;
        mRuntimeScene.Reset();
        mRuntimeScene = nullptr;
        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneHierarchyPanel.SetContext(mEditorScene);
        ScriptEngine::SetSceneContext(mEditorScene);
        Renderer::SetSceneContext(mEditorScene.Raw());
    }

    void EditorModule::OnScenePause()
    {
        mSceneState = SceneState::Pause;
    }

    void EditorModule::OnSceneResume()
    {
        mSceneState = SceneState::Play;
    }

    void EditorModule::OnUpdate(Timestep ts)
    {
        // Resize
        FramebufferSpecification spec = mFramebuffer->GetSpecification();
        if (mViewportSize.x > 0.0f && mViewportSize.y > 0.0f && (spec.Width != mViewportSize.x || spec.Height != mViewportSize.y))
        {
            mFramebuffer->Resize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
            mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
            mEditorScene->OnViewportResize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
        }
        mFramebuffer->Bind();
        mFramebuffer->Clear(mClearColor);

        RenderCommand::SetClearColor(mClearColor);
        RenderCommand::Clear();

        //Render
        switch (mSceneState)
        {
            case EditorModule::SceneState::Edit:
                mEditorCamera.OnUpdate(ts);
                mEditorScene->OnUpdateEditor(ts, mEditorCamera); break;

            case EditorModule::SceneState::Play:
                mRuntimeScene->OnUpdate(ts);
                mRuntimeScene->OnUpdateRuntime(ts); break;

            case EditorModule::SceneState::Pause:
                mRuntimeScene->OnUpdateRuntime(ts); break;
        }
        RenderCommand::BindBackbuffer();

        mFramebuffer->Unbind();
        mEditorScene->mSelectedEntity = mSceneHierarchyPanel.GetSelectedEntity();
    }

    void EditorModule::OnImGuiRender()
    {
        UI::BeginDockspace();
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::Button(ICON_ELECTRO_ARROWS) && !mGizmoInUse)
                mGizmoType = ImGuizmo::OPERATION::TRANSLATE;
            if (ImGui::Button(ICON_ELECTRO_REPEAT) && !mGizmoInUse)
                mGizmoType = ImGuizmo::OPERATION::ROTATE;
            if (ImGui::Button(ICON_ELECTRO_EXPAND) && !mGizmoInUse)
                mGizmoType = ImGuizmo::OPERATION::SCALE;
            if (ImGui::Button(ICON_ELECTRO_FLOPPY_O))
                SaveScene();

            ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() / 2.2));
            if (mSceneState == SceneState::Edit)
            {
                if (UI::ColorButton(ICON_ELECTRO_PLAY, ImVec4(0.1f, 0.8f, 0.1f, 1.0f)))
                    OnScenePlay();
                UI::ToolTip("Play the scene");

                ImGui::SameLine();
                if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                    ELECTRO_WARN("You can pause the game only in Playmode! Please enter in Playmode to pause the game.");
            }
            else if (mSceneState == SceneState::Play)
            {
                if (UI::ColorButton(ICON_ELECTRO_STOP, ImVec4(0.9f, 0.1f, 0.1f, 1.0f)))
                    OnSceneStop();
                UI::ToolTip("Stop the scene");
                ImGui::SameLine();
                if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                    OnScenePause();
                UI::ToolTip("Pause the scene");
            }
            else if (mSceneState == SceneState::Pause)
            {
                if (UI::ColorButton(ICON_ELECTRO_STOP, ImVec4(0.9f, 0.1f, 0.1f, 1.0f)))
                    OnSceneStop();
                UI::ToolTip("Stop the scene");
                ImGui::SameLine();
                if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                    OnSceneResume();
                UI::ToolTip("Resume the scene");
            }
            ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() - 21));
            if (ImGui::Button(ICON_ELECTRO_GITHUB))
                OS::OpenURL("https://github.com/FahimFuad/Electro");

            UI::ToolTip("Open Github repository of Electro");
            ImGui::EndMainMenuBar();
        }

        UI::BeginViewport(VIEWPORT_TITLE);
        auto viewportOffset = ImGui::GetCursorPos();

        if (mSceneState == SceneState::Play)
            UI::DrawRectAroundWindow({ 1.0f, 1.0f, 0.0f, 1.0f });
        else if (mSceneState == SceneState::Pause)
            UI::DrawRectAroundWindow({ 0.0f, 0.0f, 1.0f, 1.0f });

        mViewportFocused = ImGui::IsWindowFocused();
        mViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiModule()->BlockEvents(!mViewportFocused || !mViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        UI::Image(mFramebuffer->GetColorAttachmentID(0), mViewportSize);
        {
            const ImGuiPayload* data = UI::DragAndDropTarget(ELECTRO_SCENE_FILE_DND_ID);
            if (data)
            {
                InitSceneEssentials();
                SceneSerializer deSerializer(mEditorScene, this);
                String filepath = *(String*)data->Data;
                deSerializer.Deserialize(filepath);
                mActiveFilepath = filepath;
            }
        }
        {
            const ImGuiPayload* data = UI::DragAndDropTarget(MESH_DND_ID);
            if (data)
            {
                mEditorScene->CreateEntity("Mesh").AddComponent<MeshComponent>().Mesh = Mesh::Create(*(String*)data->Data);
            }
        }
        RenderGizmos();
        UI::EndViewport();

        RenderPanels();
        UI::EndDockspace();
    }

    void EditorModule::OnEvent(Event& e)
    {
        mSceneHierarchyPanel.OnEvent(e);
        if (mSceneState == SceneState::Edit)
        {
            if (mViewportHovered)
                mEditorCamera.OnEvent(e);
        }

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(ELECTRO_BIND_EVENT_FN(EditorModule::OnKeyPressed));
    }

    bool EditorModule::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 1)
            return false;

        const bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        const bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (e.GetKeyCode())
        {
            case Key::N: if (control) NewProject();  break;
            case Key::O: if (control) Open(); break;
            case Key::S: if (control) SaveScene(); if (control && shift) SaveSceneAs(); break;

            // Gizmos
            case Key::Q: if (!mGizmoInUse) mGizmoType = -1;                             break;
            case Key::W: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::TRANSLATE; break;
            case Key::E: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::ROTATE;    break;
            case Key::R: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::SCALE;     break;
            case Key::F5:
                if (mSceneState == SceneState::Edit)
                    OnScenePlay();
                else if (mSceneState == SceneState::Play)
                    OnSceneStop();
                else if (mSceneState == SceneState::Pause)
                    OnSceneResume();
                break;
        }
        return false;
    }

    void EditorModule::UpdateWindowTitle(const String& sceneName)
    {
        auto& app = Application::Get();
        const String config = app.GetBuildConfig();
        const String title = "Electro - " + sceneName + " - " + config;
        app.GetWindow().SetTitle(title);
    }

    void EditorModule::RenderGizmos()
    {
        Entity selectedEntity = mSceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && mGizmoType != -1 && mSceneState != SceneState::Play)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            const float windowWidth = ImGui::GetWindowWidth();
            const float windowHeight = ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            glm::mat4 cameraView, cameraProjection;

            if (mSceneState == SceneState::Play)
            {
                auto cameraEntity = mEditorScene->GetPrimaryCameraEntity();
                const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                cameraProjection = camera.GetProjection();
                cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
            }

            if (mSceneState == SceneState::Edit)
            {
                cameraProjection = mEditorCamera.GetProjection();
                cameraView = mEditorCamera.GetViewMatrix();
            }

            // Entity transform
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            // Snapping
            const bool snap = Input::IsKeyPressed(Key::LeftControl);
            float snapValue = 0.5f; // Snap to 0.5m for translation/scale
            // Snap to 45 degrees for rotation
            if (mGizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = { snapValue, snapValue, snapValue };
            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                static_cast<ImGuizmo::OPERATION>(mGizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                mGizmoInUse = true;
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(transform, translation, rotation, scale);

                const glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
            else
                mGizmoInUse = false;
        }
    }

    // Render all the panels
    void EditorModule::RenderPanels()
    {
        if(mShowConsolePanel)
            Console::Get()->OnImGuiRender(&mShowConsolePanel);

        if(mShowHierarchyAndInspectorPanel)
            mSceneHierarchyPanel.OnImGuiRender(&mShowHierarchyAndInspectorPanel);

        if(mShowProfilerPanel)
            mProfilerPanel.OnImGuiRender(&mShowProfilerPanel);

        if(mShowVaultAndCachePanel)
            mVaultPanel.OnImGuiRender(&mShowVaultAndCachePanel);

        if(mShowMaterialPanel)
            mMaterialPanel.OnImGuiRender(&mShowMaterialPanel, mSceneHierarchyPanel.GetSelectedEntity());

        if(mShowPhysicsSettingsPanel)
            mPhysicsSettingsPanel.OnImGuiRender(&mShowPhysicsSettingsPanel);

        if (mShowRendererSettingsPanel)
            mRendererSettingsPanel.OnImGuiRender(&mShowRendererSettingsPanel);
    }

    void EditorModule::NewProject()
    {
        const char* filepath = OS::SelectFolder("Choose a location to save the project");
        if (filepath)
        {
            InitSceneEssentials();

            // Initialize the assets path
            mAssetsPath = filepath;

            AssetManager::Init(mAssetsPath);
            FileSystem::CreateOrEnsureFolderExists(mAssetsPath, "Scenes");
            const String scenePath = mAssetsPath + "/" + "Scenes";

            //TODO: Automate this project name
            String projectName = FileSystem::GetNameWithoutExtension(filepath);
            String& sceneElectroPath = scenePath + String("/") + projectName + ".electro";
            std::ofstream stream = std::ofstream(sceneElectroPath); // Create the file

            if (stream.bad())
                ELECTRO_ERROR("Bad stream!");

            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(sceneElectroPath);
            UpdateWindowTitle(projectName);
            mActiveFilepath = sceneElectroPath + String("/") + projectName + ".electro";
        }
    }

    void EditorModule::Open()
    {
        std::optional<String> filepath = OS::OpenFile("*.electro");
        if (filepath)
        {
            mActiveFilepath = *filepath;
            InitSceneEssentials();
            SceneSerializer serializer(mEditorScene, this);
            serializer.Deserialize(*filepath);

            AssetManager::Init(mAssetsPath);
            ELECTRO_INFO("Succesfully deserialized scene!");
        }
    }

    void EditorModule::SaveSceneAs()
    {
        std::optional<String> filepath = OS::SaveFile("*.electro");
        if (filepath)
        {
            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(*filepath);
            ELECTRO_INFO("Scene serialized succesfully!");
        }
    }

    void EditorModule::SaveScene()
    {
        if (mActiveFilepath.empty())
            SaveSceneAs();
        else
        {
            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(mActiveFilepath);
            ELECTRO_INFO("Scene Saved!");
        }
    }

    void EditorModule::InitSceneEssentials()
    {
        mEditorScene.Reset();
        mEditorScene = Ref<Scene>::Create();
        Renderer::SetSceneContext(mEditorScene.Raw());
        mEditorScene->OnViewportResize((Uint)mViewportSize.x, (Uint)mViewportSize.y);
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1024.0f);
        mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
        mSceneHierarchyPanel.SetContext(mEditorScene);
    }
}
