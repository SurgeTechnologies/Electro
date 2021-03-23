//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroEditorLayer.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Utility/ElectroFileDialogs.hpp"
#include "Math/ElectroMath.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "Core/ElectroVault.hpp"
#include <FontAwesome.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    static bool s_ShowHierarchyAndInspectorPanel = true;
    static bool s_ShowConsolePanel = true;
    static bool s_ShowVaultAndCachePanel = true;
    static bool s_ShowMaterialPanel = true;

    static bool s_ShowRendererSettingsPanel = false;
    static bool s_ShowRendererProfilerPanel = false;

    EditorLayer::EditorLayer()
        : mVaultPanel(this) {}

    void EditorLayer::OnAttach()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.SwapChainTarget = false;
        fbSpec.BufferDescriptions.emplace_back(FramebufferSpecification::BufferDesc(FormatCode::R32G32B32A32_FLOAT, BindFlag::RENDER_TARGET | BindFlag::SHADER_RESOURCE));
        fbSpec.BufferDescriptions.emplace_back(FramebufferSpecification::BufferDesc(FormatCode::D24_UNORM_S8_UINT, BindFlag::DEPTH_STENCIL));
        mFramebuffer = Framebuffer::Create(fbSpec);

        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle("Spike Engine Startup Window (No project is opened)");
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnScenePlay()
    {
        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneState = SceneState::Play;

        mRuntimeScene = Ref<Scene>::Create();
        mEditorScene->CopySceneTo(mRuntimeScene);

        mRuntimeScene->OnRuntimeStart();
        mSceneHierarchyPanel.SetContext(mRuntimeScene);
    }

    void EditorLayer::OnSceneStop()
    {
        mRuntimeScene->OnRuntimeStop();
        mSceneState = SceneState::Edit;

        // Unload runtime scene
        mRuntimeScene = nullptr;

        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneHierarchyPanel.SetContext(mEditorScene);
    }

    void EditorLayer::OnScenePause()
    {
        mSceneState = SceneState::Pause;
    }

    void EditorLayer::OnSceneResume()
    {
        mSceneState = SceneState::Play;
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        // Resize
        FramebufferSpecification spec = mFramebuffer->GetSpecification();
        if ( m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            mFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            mEditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            mEditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // Render
        Renderer::UpdateStats();
        Renderer2D::UpdateStats();

        mFramebuffer->Bind();
        RenderCommand::Clear();
        mFramebuffer->Clear(mClearColor);

        switch (mSceneState)
        {
        case EditorLayer::SceneState::Edit:
            mEditorCamera.OnUpdate(ts);
            mEditorScene->OnUpdateEditor(ts, mEditorCamera); break;

        case EditorLayer::SceneState::Play:
            if (mViewportFocused) mEditorCamera.OnUpdate(ts);
            mRuntimeScene->OnUpdate(ts);
            mRuntimeScene->OnUpdateRuntime(ts); break;

        case EditorLayer::SceneState::Pause:
            if (mViewportFocused) mEditorCamera.OnUpdate(ts);
            mRuntimeScene->OnUpdateRuntime(ts); break;
        }
        RenderCommand::BindBackbuffer();
        mFramebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
        GUI::BeginDockspace();
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Folder", "CTRL+O"))
                    OpenFolder();

                if (ImGui::MenuItem("Open Scene"))
                    OpenScene();

                if (ImGui::MenuItem("New", "CTRL+N"))
                    NewScene();

                if (ImGui::MenuItem("Save", "CTRL+S"))
                    SaveScene();

                if (ImGui::MenuItem("Save As...", "CTRL+SHIFT+S"))
                    SaveSceneAs();

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Inspector and Hierarchy"))
                    s_ShowHierarchyAndInspectorPanel = true;

                if (ImGui::MenuItem("Console"))
                    s_ShowConsolePanel = true;

                if (ImGui::MenuItem("Vault and SpikeCache"))
                    s_ShowVaultAndCachePanel = true;

                if (ImGui::MenuItem("Material Inspector"))
                    s_ShowMaterialPanel = true;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Renderer"))
            {
                if (ImGui::MenuItem("Settings"))
                    s_ShowRendererSettingsPanel = true;

                if (ImGui::MenuItem("Profiler"))
                    s_ShowRendererProfilerPanel = true;

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        RenderPanels();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5, 0.5, 0.5, 1.0f));
        ImGui::Begin("ToolBar", false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

        if (ImGui::Button(ICON_FK_FLOPPY_O)) SaveScene();
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2);
        if (mSceneState == SceneState::Edit)
        {
            if (ImGui::ArrowButton("Play", ImGuiDir_Right))
                OnScenePlay();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_PAUSE))
                ELECTRO_WARN("You can pause the scene only in Playmode!");
        }
        else if (mSceneState == SceneState::Play)
        {
            if (ImGui::Button(ICON_FK_STOP))
                OnSceneStop();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_PAUSE))
                OnScenePause();
        }
        else if (mSceneState == SceneState::Pause)
        {
            if (ImGui::Button(ICON_FK_STOP))
                OnSceneStop();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_PAUSE))
                OnSceneResume();
        }
        ImGui::End();
        ImGui::PopStyleColor(3);

        GUI::BeginViewport(ICON_FK_GAMEPAD" Viewport");
        auto viewportOffset = ImGui::GetCursorPos();

        if (mSceneState == SceneState::Play)
            DrawRectAroundWindow({ 1.0f, 1.0f, 0.0f, 1.0f });
        else if (mSceneState == SceneState::Pause)
            DrawRectAroundWindow({ 0.0f, 0.0f, 1.0f, 1.0f });

        mViewportFocused = ImGui::IsWindowFocused();
        mViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!mViewportFocused || !mViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        GUI::DrawImageControl(mFramebuffer->GetColorViewID(), m_ViewportSize);
        RenderGizmos();
        GUI::EndViewport();

        if (s_ShowRendererSettingsPanel)
        {
            ImGui::Begin("Renderer Settings", &s_ShowRendererSettingsPanel);
            GUI::DrawColorControl4("Clear Color", mClearColor);
            ImGui::Separator();

            if (ImGui::TreeNodeEx("Configure SKYBOX", ImGuiTreeNodeFlags_OpenOnArrow))
            {
                ImGui::TextColored({ 0.1f, 0.9f, 0.1f, 1.0f }, ICON_FK_ARROW_DOWN " IMPORTANT notes regarding Skybox " ICON_FK_ARROW_DOWN);
                ImGui::BulletText("Remember the folder must contain 6 exactly image files, nothing else");
                ImGui::BulletText("The image files must be named as \"Aright, Bleft, Ctop, Dbottom, Efront, Fback\"");
                ImGui::BulletText("The names represents the 6 sides of a SKYBOX");
                ImGui::BulletText("Yes, the prefix A, B, C, D, E, F in front of the image file names are necessary!");
                GUI::DrawDynamicToggleButton(ICON_FK_TIMES, ICON_FK_CHECK, { 0.7f, 0.1f, 0.1f, 1.0f }, { 0.2f, 0.5f, 0.2f, 1.0f }, &Renderer::GetSkyboxActivationBool());
                GUI::DrawToolTip("Use Skybox");
                ImGui::SameLine();

                if (ImGui::Button("Open Skybox"))
                {
                    const char* folderpath = FileDialogs::SelectFolder("Open A folder containing skybox");
                    if (folderpath)
                    {
                        mCurrentSkyboxPath = folderpath;
                        Renderer::SetSkybox(Skybox::Create(TextureCube::Create(mCurrentSkyboxPath)));
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Generate Skybox"))
                {
                    if (!mCurrentSkyboxPath.empty())
                        Renderer::SetSkybox(Skybox::Create(TextureCube::Create(mCurrentSkyboxPath)));
                    else
                        ELECTRO_WARN("Select a skybox path first via 'Open Skybox' button!");
                }
                ImGui::SameLine();
                if (ImGui::Button("Destroy Skybox"))
                    Renderer::SetSkybox(nullptr); //Destroy the skybox

                ImGui::PushItemWidth(-1);
                ImGui::Text("Skybox folderpath:");
                ImGui::SameLine();
                ImGui::InputText("##skyboxFilepath", (char*)mCurrentSkyboxPath.c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::TreePop();
            }
            ImGui::Separator();
            ImGui::End();
        }

        GUI::EndDockspace();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        mSceneHierarchyPanel.OnEvent(e);
        if (mSceneState == SceneState::Edit)
        {
            if (mViewportHovered)
                mEditorCamera.OnEvent(e);
            mEditorScene->OnEvent(e);
        }
        else if (mSceneState == SceneState::Play)
            mRuntimeScene->OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(ELECTRO_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (e.GetKeyCode())
        {
            case Key::N: if (control) NewScene();   break;
            case Key::O: if (control) OpenFolder(); break;
            case Key::S: if (control) SaveScene(); if (control && shift) SaveSceneAs(); break;

            // Gizmos
            case Key::Q: if (!mGizmoInUse) mGizmoType = -1;                             break;
            case Key::W: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::TRANSLATE; break;
            case Key::E: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::ROTATE;    break;
            case Key::R: if (!mGizmoInUse) mGizmoType = ImGuizmo::OPERATION::SCALE;     break;
        }
        return false;
    }

    void EditorLayer::UpdateWindowTitle(const String& sceneName)
    {
        String title = "Electro Editor " + sceneName;
        Application::Get().GetWindow().SetTitle(title);
    }

    void EditorLayer::DrawRectAroundWindow(const glm::vec4& color)
    {
        ImVec2 windowMin = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 windowMax = { windowMin.x + windowSize.x, windowMin.y + windowSize.y };
        ImGui::GetForegroundDrawList()->AddRect(windowMin, windowMax, ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    }

    void EditorLayer::RenderGizmos()
    {
        Entity selectedEntity = mSceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && mGizmoType != -1 && mSceneState != SceneState::Play)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
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
            bool snap = Input::IsKeyPressed(Key::LeftControl);
            float snapValue = 0.5f; // Snap to 0.5m for translation/scale
            // Snap to 45 degrees for rotation
            if (mGizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = { snapValue, snapValue, snapValue };
            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)mGizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                mGizmoInUse = true;
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(transform, translation, rotation, scale);

                glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
            else
                mGizmoInUse = false;
        }
    }

    void EditorLayer::RenderPanels()
    {
        if(s_ShowConsolePanel)
            Console::Get()->OnImGuiRender(&s_ShowConsolePanel);

        if(s_ShowHierarchyAndInspectorPanel)
            mSceneHierarchyPanel.OnImGuiRender(&s_ShowHierarchyAndInspectorPanel);

        if(s_ShowRendererProfilerPanel)
            mProfilerPanel.OnImGuiRender(&s_ShowRendererProfilerPanel);

        if(s_ShowVaultAndCachePanel)
            mVaultPanel.OnImGuiRender(&s_ShowVaultAndCachePanel);

        if(s_ShowMaterialPanel)
            mMaterialPanel.OnImGuiRender(&s_ShowMaterialPanel, mSceneHierarchyPanel.GetSelectedEntity());
    }

    // File Stuff
    void EditorLayer::NewScene()
    {
        const char* filepath = FileDialogs::SelectFolder("Select a location to save project files");
        if (filepath)
        {
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            mSceneHierarchyPanel.SetContext(mEditorScene);

            mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
            mEditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

            mFirstTimeSave = false;
            Vault::Init(filepath);

            String projectName = OS::GetNameWithoutExtension(filepath);
            mActiveFilepath = String(filepath) + "/" + projectName + ".spike";

            SceneSerializer serializer(mEditorScene);
            serializer.Serialize(mActiveFilepath);
            UpdateWindowTitle(projectName);
        }
    }

    void EditorLayer::OpenFolder()
    {
        const char* filepath = FileDialogs::SelectFolder("Select a folder to open");
        if (filepath)
        {
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            mSceneHierarchyPanel.SetContext(mEditorScene);

            mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
            mEditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

            Vault::Init(filepath);

            String projectName = OS::GetNameWithoutExtension(filepath);
            UpdateWindowTitle(projectName);
        }
    }

    void EditorLayer::OpenScene()
    {
        const char* pattern[1] = { "*.spike" };
        const char* filepath = FileDialogs::OpenFile("Open Scene", "", 1, pattern, "", false);
        if (filepath)
        {
            mFirstTimeSave = false;
            mActiveFilepath = filepath;
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            mSceneHierarchyPanel.SetContext(mEditorScene);

            SceneSerializer serializer(mEditorScene);
            serializer.Deserialize(filepath);
            ELECTRO_INFO("Succesfully deserialized scene!");
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        const char* pattern[1] = { "*.spike" };
        const char* filepath = FileDialogs::SaveFile("Save Scene", "Scene.spike", 1, pattern, "Spike Scene");
        if (filepath)
        {
            mFirstTimeSave = false;
            SceneSerializer serializer(mEditorScene);
            serializer.Serialize(filepath);
            ELECTRO_INFO("Scene serialized succesfully!");
        }
    }

    void EditorLayer::SaveScene()
    {
        if (mFirstTimeSave)
            SaveSceneAs();
        else
        {
            SceneSerializer serializer(mEditorScene);
            serializer.Serialize(mActiveFilepath);
            ELECTRO_INFO("Scene Saved!");
        }
    }
}
