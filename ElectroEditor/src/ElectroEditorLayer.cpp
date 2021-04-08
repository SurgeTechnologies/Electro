//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroEditorLayer.hpp"
#include "Core/ElectroVault.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroSceneRenderer.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "Math/ElectroMath.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include <FontAwesome.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Electro
{
    static bool sShowHierarchyAndInspectorPanel = true;
    static bool sShowConsolePanel               = true;
    static bool sShowVaultAndCachePanel         = true;
    static bool sShowMaterialPanel              = true;
    static bool sShowRendererSettingsPanel      = false;
    static bool sShowRendererProfilerPanel      = false;
    static bool sShowPhysicsSettingsPanel       = false;

    EditorLayer::EditorLayer()
        : mVaultPanel(this)
    {
        auto texture = Texture2D::Create("Resources/ThirdParty/physx.png");
        Vault::Submit<Texture2D>(texture);
        mPhysicsSettingsPanel.Init();
    }

    void EditorLayer::OnAttach()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::R32G32B32A32_FLOAT, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        mFramebuffer = Framebuffer::Create(fbSpec);

        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle(" - " + Application::Get().GetBuildConfig());
    }

    void EditorLayer::OnDetach() {}

    void EditorLayer::OnScenePlay()
    {
        ScriptEngine::SetSceneContext(mEditorScene);
        ScriptEngine::ReloadAssembly(Application::Get().GetCSharpDLLPath());

        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneState = SceneState::Play;

        mRuntimeScene = Ref<Scene>::Create(true);
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
        Renderer::UpdateStatus();
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
        mEditorScene->mSelectedEntity = mSceneHierarchyPanel.GetSelectedEntity();
    }

    void EditorLayer::OnImGuiRender()
    {
        UI::BeginDockspace();
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
                    sShowHierarchyAndInspectorPanel = true;

                if (ImGui::MenuItem("Console"))
                    sShowConsolePanel = true;

                if (ImGui::MenuItem("Vault"))
                    sShowVaultAndCachePanel = true;

                if (ImGui::MenuItem("Material Inspector"))
                    sShowMaterialPanel = true;

                if (ImGui::MenuItem("Physics Settings"))
                    sShowPhysicsSettingsPanel = true;

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Renderer"))
            {
                if (ImGui::MenuItem("Settings"))
                    sShowRendererSettingsPanel = true;

                if (ImGui::MenuItem("Profiler"))
                    sShowRendererProfilerPanel = true;

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        RenderPanels();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5, 0.5, 0.5, 1.0f));
        ImGui::Begin("ToolBar", false, ImGuiWindowFlags_NoDecoration);

        if (ImGui::Button(ICON_ELECTRO_FLOPPY_O)) SaveScene();
        ImGui::SameLine();

        ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() / 2.2)); //Approximation, trying to draw at the middle
        if (mSceneState == SceneState::Edit)
        {
            if (UI::ColorButton(ICON_ELECTRO_PLAY, ImVec4(0.1f, 0.8f, 0.1f, 1.0f)))
                OnScenePlay();
            ImGui::SameLine();
            if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                ELECTRO_WARN("You can pause the game only in Playmode! Please enter in Playmode to pause the game.");
        }
        else if (mSceneState == SceneState::Play)
        {
            if (UI::ColorButton(ICON_ELECTRO_STOP, ImVec4(0.9f, 0.1f, 0.1f, 1.0f)))
                OnSceneStop();
            ImGui::SameLine();
            if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                OnScenePause();
        }
        else if (mSceneState == SceneState::Pause)
        {
            if (UI::ColorButton(ICON_ELECTRO_STOP, ImVec4(0.9f, 0.1f, 0.1f, 1.0f)))
                OnSceneStop();
            ImGui::SameLine();
            if (UI::ColorButton(ICON_ELECTRO_PAUSE, ImVec4(0.0980f, 0.46667f, 0.790196f, 1.0f)))
                OnSceneResume();
        }
        ImGui::End();
        ImGui::PopStyleColor(3);

        UI::BeginViewport(ICON_ELECTRO_GAMEPAD" Viewport");
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

        UI::Image(mFramebuffer->GetColorAttachmentID(0), m_ViewportSize);
        RenderGizmos();
        UI::EndViewport();
        if (sShowRendererSettingsPanel)
        {
            ImGui::Begin("Renderer Settings", &sShowRendererSettingsPanel);
            UI::Color4("Clear Color", mClearColor);
            ImGui::Separator();

            if (ImGui::TreeNodeEx("Configure SKYBOX", ImGuiTreeNodeFlags_OpenOnArrow))
            {
                ImGui::TextColored({ 0.1f, 0.9f, 0.1f, 1.0f }, ICON_ELECTRO_ARROW_DOWN" IMPORTANT notes regarding Skybox " ICON_ELECTRO_ARROW_DOWN);
                ImGui::TextUnformatted("1) The folder must contain exactly 6 image files, nothing else!"
                                     "\n2) The image files must be named as \"Aright, Bleft, Ctop, Dbottom, Efront, Fback.\""
                                     "\n3) The names represents the 6 sides of a skybox."
                                     "\n4) Yes, the prefix A, B, C, D, E, F in front of the image file names are necessary!.");

                UI::ToggleButton("Skybox", &SceneRenderer::GetSkyboxActivationBool());
                UI::ToolTip("Use Skybox");

                ImGui::SameLine();

                if (ImGui::Button("Open Skybox"))
                {
                    const char* folderpath = OS::SelectFolder("Open A folder containing skybox");
                    if (folderpath)
                    {
                        mCurrentSkyboxPath = folderpath;
                        SceneRenderer::SetSkybox(Skybox::Create(TextureCube::Create(mCurrentSkyboxPath)));
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Generate Skybox"))
                {
                    if (!mCurrentSkyboxPath.empty())
                        SceneRenderer::SetSkybox(Skybox::Create(TextureCube::Create(mCurrentSkyboxPath)));
                    else
                        ELECTRO_WARN("Select a skybox path first via 'Open Skybox' button!");
                }
                ImGui::SameLine();
                if (ImGui::Button("Destroy Skybox"))
                    SceneRenderer::SetSkybox(nullptr); //Destroy the skybox

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
        UI::EndDockspace();
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
        if (e.GetRepeatCount() > 1)
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
        if(sShowConsolePanel)
            Console::Get()->OnImGuiRender(&sShowConsolePanel);

        if(sShowHierarchyAndInspectorPanel)
            mSceneHierarchyPanel.OnImGuiRender(&sShowHierarchyAndInspectorPanel);

        if(sShowRendererProfilerPanel)
            mProfilerPanel.OnImGuiRender(&sShowRendererProfilerPanel);

        if(sShowVaultAndCachePanel)
            mVaultPanel.OnImGuiRender(&sShowVaultAndCachePanel);

        if(sShowMaterialPanel)
            mMaterialPanel.OnImGuiRender(&sShowMaterialPanel, mSceneHierarchyPanel.GetSelectedEntity());

        if(sShowPhysicsSettingsPanel)
            mPhysicsSettingsPanel.OnImGuiRender(&sShowPhysicsSettingsPanel);
    }

    // File Stuff
    void EditorLayer::NewScene()
    {
        const char* filepath = OS::SelectFolder("Select a location to save project files");
        if (filepath)
        {
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((Uint)m_ViewportSize.x, (Uint)m_ViewportSize.y);
            mSceneHierarchyPanel.SetContext(mEditorScene);

            mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
            mEditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

            mFirstTimeSave = false;
            Vault::Init(filepath);

            String projectName = OS::GetNameWithoutExtension(filepath);
            mActiveFilepath = String(filepath) + "/" + projectName + ".electro";

            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(mActiveFilepath);
            UpdateWindowTitle(projectName);
        }
    }

    void EditorLayer::OpenFolder()
    {
        const char* filepath = OS::SelectFolder("Select a folder to open");
        if (filepath)
        {
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((Uint)m_ViewportSize.x, (Uint)m_ViewportSize.y);
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
        auto filepath = OS::OpenFile("ElectroFile (*.electro)\0*.electro;");
        if (filepath)
        {
            mFirstTimeSave = false;
            mActiveFilepath = *filepath;
            mEditorScene = Ref<Scene>::Create();
            mEditorScene->OnViewportResize((Uint)m_ViewportSize.x, (Uint)m_ViewportSize.y);
            mSceneHierarchyPanel.SetContext(mEditorScene);

            SceneSerializer serializer(mEditorScene, this);
            serializer.Deserialize(*filepath);
            ELECTRO_INFO("Succesfully deserialized scene!");
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        auto filepath = OS::SaveFile("ElectroFile (*.electro)\0*.electro;");
        if (filepath)
        {
            mFirstTimeSave = false;
            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(*filepath);
            ELECTRO_INFO("Scene serialized succesfully!");
        }
    }

    void EditorLayer::SaveScene()
    {
        if (mFirstTimeSave)
            SaveSceneAs();
        else
        {
            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(mActiveFilepath);
            ELECTRO_INFO("Scene Saved!");
        }
    }
}
