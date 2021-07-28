//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include <Electro.hpp>
#include "EditorModule.hpp"
#include "UIUtils/UIUtils.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include <fmt/core.h>

namespace Electro
{
    EditorModule::EditorModule()
        : mAssetsPanel(this)
    {
        memset(mInputBuffer, 0, INPUT_BUFFER_LENGTH);
        memset(mNameBuffer, 0, INPUT_BUFFER_LENGTH);
        memset(mSceneNameBuffer, 0, INPUT_BUFFER_LENGTH);
    }

    void EditorModule::Init()
    {
        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1024.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle("Null");

        ScriptEngine::SetSceneContext(mEditorScene);
        Renderer::SetSceneContext(mEditorScene.Raw());

        // 9 Panels in total (SceneHierarchyPanel contains Inspector)
        PanelManager::PushPanel(PROJECT_SETTINGS_TITLE, &mProjectSettingsPanel, &mShowProjectSettingsPanel, nullptr);
        PanelManager::PushPanel(PROFILER_TITLE, &mProfilerPanel, &mShowProfilerPanel, nullptr);
        PanelManager::PushPanel(ASSET_REGISTRY_TITLE, &mAssetRegistryPanel, &mShowAssetRegistryPanel, nullptr);
        PanelManager::PushPanel(ASSETS_TITLE, &mAssetsPanel, &mShowAssetsPanel, this);
        PanelManager::PushPanel(PHYSICS_SETTINGS_TITLE, &mPhysicsSettingsPanel, &mShowPhysicsSettingsPanel, nullptr);
        PanelManager::PushPanel(RENDERER_SETTINGS_TITLE, &mRendererSettingsPanel, &mShowRendererSettingsPanel, nullptr);
        PanelManager::PushPanel(MATERIAL_INSPECTOR_TITLE, &mMaterialPanel, &mShowMaterialPanel, &mSceneHierarchyPanel);
        PanelManager::PushPanel(HIERARCHY_TITLE" & Inspector", &mSceneHierarchyPanel, &mShowHierarchyAndInspectorPanel, nullptr);

        // Dummy Project
        mActiveProject = Ref<Project>::Create();
        ProjectManager::SetActive(mActiveProject);
    }

    void EditorModule::OnScenePlay()
    {
        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneState = SceneState::Play;

        ScriptEngine::ReloadAssembly(Application::Get().GetCSharpDLLPath());

        mRuntimeScene = Ref<Scene>::Create(true);
        mEditorScene->CopySceneTo(mRuntimeScene);
        mRuntimeScene->OnRuntimeStart();

        mSceneHierarchyPanel.SetContext(mRuntimeScene);
        Renderer::SetSceneContext(mRuntimeScene.Raw());
    }

    void EditorModule::OnSceneStop()
    {
        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneState = SceneState::Edit;

        mRuntimeScene->OnRuntimeStop();
        mRuntimeScene.Reset();

        mSceneHierarchyPanel.SetContext(mEditorScene);
        Renderer::SetSceneContext(mEditorScene.Raw());
        ScriptEngine::SetSceneContext(mEditorScene);
    }

    void EditorModule::OnScenePause()
    {
        mSceneState = SceneState::Pause;
    }

    void EditorModule::OnSceneResume()
    {
        mSceneState = SceneState::Play;
    }

    void EditorModule::SerializeScene(const String& path)
    {
        mActiveFilepath = path;
        mActiveSceneName = FileSystem::GetNameWithoutExtension(mActiveFilepath);
        SceneSerializer serializer(mEditorScene);
        serializer.Serialize(mActiveFilepath);
    }

    void EditorModule::DeserializeScene(const String& path)
    {
        mActiveFilepath = path;
        mActiveSceneName = FileSystem::GetNameWithoutExtension(mActiveFilepath);
        SceneSerializer deserializer(mEditorScene);
        deserializer.Deserialize(mActiveFilepath);
    }

    void EditorModule::OnUpdate(Timestep ts)
    {
        // Resize
        Ref<Renderbuffer>& framebuffer = Renderer::GetFinalPassTexture();
        const RenderbufferSpecification spec = framebuffer->GetSpecification();
        if (mViewportSize.x > 0.0f && mViewportSize.y > 0.0f && (spec.Width != mViewportSize.x || spec.Height != mViewportSize.y))
        {
            framebuffer->Resize(static_cast<Uint>(mViewportSize.x), static_cast<Uint>(mViewportSize.y));
            mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
            mEditorScene->OnViewportResize(static_cast<Uint>(mViewportSize.x), static_cast<Uint>(mViewportSize.y));
        }

        RenderCommand::SetClearColor(mClearColor);
        RenderCommand::Clear();

        //Render
        switch (mSceneState)
        {
            case SceneState::Edit:
                mEditorCamera.OnUpdate(ts);
                mEditorScene->OnUpdateEditor(ts, mEditorCamera); break;

            case SceneState::Play:
                mRuntimeScene->OnUpdate(ts);
                mRuntimeScene->OnUpdateRuntime(ts); break;

            case SceneState::Pause:
                mRuntimeScene->OnUpdateRuntime(ts); break;
        }
        RenderCommand::BindBackbuffer();

        mEditorScene->SetSelectedEntity(mSceneHierarchyPanel.GetSelectedEntity());
        UpdateWindowTitle(mActiveProject->GetConfig().ProjectName);
    }

    void EditorModule::OnImGuiRender()
    {
        UI::BeginDockspace();
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project", "CTRL + N"))
                    mOpenNewProjectWindow = true;

                if (ImGui::MenuItem("Open Project", "CTRL + O"))
                    OpenProject();

                if (ImGui::MenuItem("Save", "CTRL + S"))
                    SaveScene();

                if (ImGui::MenuItem("Save As", "CTRL + SHIFT + S"))
                    SaveSceneAs();

                if (ImGui::MenuItem("Export"))
                    mOpenExportProjectWindow = true;

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();
                ImGui::EndMenu();
            }

            // New Project
            if (mOpenNewProjectWindow)
            {
                ImGui::OpenPopup("New Project");
                mOpenNewProjectWindow = false;
            } NewProject();

            if (mOpenExportProjectWindow)
            {
                ImGui::OpenPopup("Export Project");
                mOpenExportProjectWindow = false;
            } ExportProject();

            if (ImGui::BeginMenu("View"))
            {
                PanelMap& panelMap = PanelManager::GetPanelMap();
                for (auto& [name, panel] : panelMap)
                {
                    if (ImGui::MenuItem(name.c_str()))
                    {
                        *panel.Data1 = true;
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() / 2.2));
            if (mSceneState == SceneState::Edit)
            {
                if (UI::ColorButton(ICON_ELECTRO_PLAY, ImVec4(0.1f, 0.8f, 0.1f, 1.0f)))
                    OnScenePlay();
                UI::ToolTip("Play the scene");

                ImGui::SameLine();
                if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                    Log::Warn("You can pause the game only in Playmode! Please enter in Playmode to pause the game.");
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
            ImGui::EndMainMenuBar();
        }

        UI::BeginViewport(VIEWPORT_TITLE);
        const ImVec2 corner = ImGui::GetCursorPos();

        if (mSceneState == SceneState::Play)
            UI::DrawRectAroundWindow({ 1.0f, 1.0f, 0.0f, 1.0f });
        else if (mSceneState == SceneState::Pause)
            UI::DrawRectAroundWindow({ 0.0f, 0.0f, 1.0f, 1.0f });

        mViewportFocused = ImGui::IsWindowFocused();
        mViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiModule()->BlockEvents(!mViewportFocused || !mViewportHovered);

        const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        UI::Image(Renderer::GetFinalPassTexture()->GetColorAttachmentID(0), mViewportSize);
        // Handle stuff dropped in viewport
        {
            const ImGuiPayload* data = UI::DragAndDropTarget(ELECTRO_SCENE_FILE_DND_ID);
            if (data)
            {
                InitSceneEssentials();
                const String filepath = *static_cast<String*>(data->Data);
                DeserializeScene(filepath);
            }
        }
        {
            const ImGuiPayload* data = UI::DragAndDropTarget(MESH_DND_ID);
            if (data)
                mEditorScene->CreateEntity("Mesh").AddComponent<MeshComponent>().Mesh = Mesh::Create(*(String*)data->Data);
        }

        RenderGizmos();
        UI::EndViewport();

        PanelManager::RenderAllPanels();
        //ScriptEngine::OnImGuiRender();
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
            case Key::N: if (control) mOpenNewProjectWindow = true;  break;
            case Key::O: if (control) OpenProject(); break;
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
            case Key::F:
                if (!mSceneHierarchyPanel.GetSelectedEntity())
                    break;
                mEditorCamera.Focus(mSceneHierarchyPanel.GetSelectedEntity().Transform().Translation);
                break;
            default: break;
        }
        return false;
    }

    void EditorModule::UpdateWindowTitle(const String& projectName)
    {
        Application& app = Application::Get();
        const String& config = app.GetBuildConfig();
        const String& title = fmt::format("Electro <{0} - {1}> - {2}", projectName, mActiveSceneName, config);
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

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), static_cast<ImGuizmo::OPERATION>(mGizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);
            //ImGuizmo::ViewManipulate(glm::value_ptr(mEditorCamera.GetViewMatrix()), mEditorCamera.GetDistance(), ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(128, 128), 0x10101010);

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

    void EditorModule::NewProject()
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2{ 400,0 });

        if (ImGui::BeginPopupModal("New Project"))
        {
            ImGui::InputText("Project Path", mInputBuffer, INPUT_BUFFER_LENGTH);
            ImGui::SameLine();
            if (ImGui::Button("Open"))
            {
                const char* filepath = OS::SelectFolder("Choose a location to save the project");
                strcpy_s(mInputBuffer, INPUT_BUFFER_LENGTH, filepath);
            }
            ImGui::InputText("Project name", mNameBuffer, INPUT_BUFFER_LENGTH);
            ImGui::InputText("Scene name", mSceneNameBuffer, INPUT_BUFFER_LENGTH);

            if (ImGui::Button("Create New Project!"))
            {
                const String scenePath = "Scenes/" + String(mSceneNameBuffer) + ".electro";

                ProjectConfig config;
                config.ProjectDirectory = mInputBuffer;
                config.ScenePaths.push_back(scenePath);
                config.ProjectName = mNameBuffer;
                mActiveProject = Ref<Project>::Create(config);
                ProjectManager::SetActive(mActiveProject);

                InitSceneEssentials();
                SerializeScene(fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), scenePath));
                UpdateWindowTitle(config.ProjectName);

                memset(mInputBuffer, 0, INPUT_BUFFER_LENGTH);
                memset(mNameBuffer, 0, INPUT_BUFFER_LENGTH);
                memset(mSceneNameBuffer, 0, INPUT_BUFFER_LENGTH);

                // Serialize the project
                const ProjectSerializer projectSerializer;
                projectSerializer.Serialize(mActiveProject.Raw());
                mAssetsPanel.Load();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void EditorModule::OpenProject()
    {
        std::optional<String> filepath = OS::OpenFile("Electro Project (*.eproj)\0*.eproj\0");
        if (filepath)
        {
            const ProjectSerializer projectDeserializer;

            if (projectDeserializer.Deserialize(mActiveProject.Raw(), *filepath))
                ProjectManager::SetActive(mActiveProject);
            else
                Log::Error("Corrupted project file %s!", filepath->c_str());

            const ProjectConfig& config = mActiveProject->GetConfig();

            InitSceneEssentials();
            UpdateWindowTitle(config.ProjectName);

            if (!config.ScenePaths.empty())
                DeserializeScene(fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), config.ScenePaths[0]));

            mAssetsPanel.Load();
        }
    }

    void EditorModule::SaveSceneAs()
    {
        std::optional<String> filepath = OS::SaveFile("Electro Scene (*.electro)\0*.electro\0");
        if (filepath)
        {
            SerializeScene(*filepath);
            Log::Info("Scene serialized succesfully!");
        }
    }

    void EditorModule::ExportProject()
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 0));

        if (ImGui::BeginPopupModal("Export Project"))
        {
            if (ImGui::BeginTable("ExpTable", 2))
            {
                ImGui::TableSetupColumn("##ExpCol", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 90.0f);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Export Path");
                ImGui::TableNextColumn();
                ImGui::InputText("##ExpPath", mInputBuffer, INPUT_BUFFER_LENGTH);
                ImGui::SameLine();
                if (ImGui::Button("Open"))
                {
                    const char* filepath = OS::SelectFolder("Choose a location to export your game");
                    if (filepath)
                    {
                        memset(mInputBuffer, 0, INPUT_BUFFER_LENGTH);
                        memset(mNameBuffer, 0, INPUT_BUFFER_LENGTH);

                        strcpy_s(mInputBuffer, INPUT_BUFFER_LENGTH, filepath);
                        strcpy_s(mNameBuffer, INPUT_BUFFER_LENGTH, FileSystem::GetNameWithoutExtension(mInputBuffer).c_str());
                    }
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Executable Name");
                ImGui::TableNextColumn();
                ImGui::InputText("##AppName", mNameBuffer, INPUT_BUFFER_LENGTH);

                ImGui::EndTable();
            }

            if (mInputBuffer[0] != NULL)
            {
                if (ImGui::Button("Export!"))
                {
                    ExporterOptions options;
                    options.ExportPath = mInputBuffer;
                    options.ApplicationName = mNameBuffer;

                    RuntimeExporter::ExportCurrent(options);
                    memset(mInputBuffer, 0, INPUT_BUFFER_LENGTH);
                    memset(mNameBuffer, 0, INPUT_BUFFER_LENGTH);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
            }

            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            if (Input::IsKeyPressed(Key::Escape))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void EditorModule::SaveScene()
    {
        if (mActiveFilepath.empty())
            SaveSceneAs();
        else
        {
            SerializeScene(mActiveFilepath);
            Log::Info("Scene Saved!");
        }
    }

    void EditorModule::InitSceneEssentials()
    {
        mEditorScene.Reset();
        mEditorScene = Ref<Scene>::Create();

        mEditorScene->OnViewportResize(static_cast<Uint>(mViewportSize.x), static_cast<Uint>(mViewportSize.y));
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1024.0f);
        mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
        mSceneHierarchyPanel.SetContext(mEditorScene);

        Renderer::SetSceneContext(mEditorScene.Raw());
        ScriptEngine::SetSceneContext(mEditorScene);
    }
}
