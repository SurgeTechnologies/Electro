//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include <Electro.hpp>
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
    EditorLayer::EditorLayer()
        : mVaultPanel(this)
    {
        Vault::Submit<Texture2D>(EDevice::CreateTexture2D("Resources/ThirdParty/physx.png"));
        Vault::Submit<Texture2D>(EDevice::CreateTexture2D("Electro/assets/textures/Prototype.png"));
        mPhysicsSettingsPanel.Init();
        mMaterialPanel.Init();
    }

    void EditorLayer::OnAttach()
    {
        Application::Get().GetWindow().RegisterEditorLayer(this);
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.SwapChainTarget = false;
        fbSpec.Name = "EditorLayerFramebuffer";
        mFramebuffer = EDevice::CreateFramebuffer(fbSpec);
        Vault::Submit<Framebuffer>(mFramebuffer);

        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle("<Null Project>");
        ScriptEngine::SetSceneContext(mEditorScene);
    }

    void EditorLayer::OnDetach() {}

    void EditorLayer::OnScenePlay()
    {
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
        mRuntimeScene.Reset();
        mRuntimeScene = nullptr;
        mSceneHierarchyPanel.ClearSelectedEntity();
        mSceneHierarchyPanel.SetContext(mEditorScene);
        ScriptEngine::SetSceneContext(mEditorScene);
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
        if ( mViewportSize.x > 0.0f && mViewportSize.y > 0.0f && (spec.Width != mViewportSize.x || spec.Height != mViewportSize.y))
        {
            mFramebuffer->Resize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
            mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
            mEditorScene->OnViewportResize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
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

            ImGui::EndMainMenuBar();
        }

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
        mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        UI::Image(mFramebuffer->GetColorAttachmentID(0), mViewportSize);
        RenderGizmos();
        UI::EndViewport();
        if (mShowRendererSettingsPanel)
        {
            ImGui::Begin("Renderer Settings", &mShowRendererSettingsPanel);
            UI::Color4("Clear Color", mClearColor);
            if (ImGui::CollapsingHeader("Environment"))
            {
                ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV;
                ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

                if (ImGui::BeginTable("SkyLightTable", 2, flags))
                {
                    ImGui::TableSetupColumn("##col1", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableSetupColumn("##col2", ImGuiTableColumnFlags_WidthFixed, contentRegionAvailable.x * 0.6156f);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("File Path");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-1);
                    Ref<EnvironmentMap>& environmentMap = SceneRenderer::GetEnvironmentMapSlot();
                    if (environmentMap && !environmentMap->GetFilePath().empty())
                        ImGui::InputText("##envfilepath", (char*)environmentMap->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                    else
                        ImGui::InputText("##envfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);
                    ImGui::EndTable();

                    if (ImGui::Button("Open"))
                    {
                        std::optional<String> filepath = OS::OpenFile("*.hdr");
                        if (filepath)
                            environmentMap = Ref<EnvironmentMap>::Create(*filepath);
                    }
                    if (environmentMap)
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Remove"))
                        {
                            // Unbind the Irradiance & Prefilter Map
                            environmentMap->GetCubemap()->Unbind(5);
                            environmentMap->GetCubemap()->Unbind(6);
                            environmentMap.Reset();
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##UseEnvMap", &SceneRenderer::GetEnvironmentMapActivationBool()))
                        {
                            if (!SceneRenderer::GetEnvironmentMapActivationBool())
                            {
                                environmentMap->GetCubemap()->Unbind(5);
                                environmentMap->GetCubemap()->Unbind(6);
                            }
                            else
                            {
                                environmentMap->GetCubemap()->BindIrradianceMap(5);
                                environmentMap->GetCubemap()->BindPreFilterMap(6);
                            }
                        }
                        UI::ToolTip("Use Environment Map");
                    }
                }
            }
            ImGui::End();
        }

        RenderPanels();
        UI::EndDockspace();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        mSceneHierarchyPanel.OnEvent(e);
        if (mSceneState == SceneState::Edit)
        {
            if (mViewportHovered)
                mEditorCamera.OnEvent(e);
        }

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
            case Key::N: if (control) NewProject();  break;
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
        }
        return false;
    }

    void EditorLayer::UpdateWindowTitle(const String& sceneName)
    {
        auto& app = Application::Get();
        String config = app.GetBuildConfig();
        String title = "Electro - " + sceneName + " - " + config;
        app.GetWindow().SetTitle(title);
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
    }

    void EditorLayer::NewProject()
    {
        const char* filepath = OS::SelectFolder("Select or create a folder save project files");
        if (filepath)
        {
            InitSceneEssentials();
            Vault::Init(filepath);

            //TODO: Automate this project name
            String projectName = OS::GetNameWithoutExtension(filepath);
            mActiveFilepath = filepath + String("/") + projectName + ".electro";
            std::ofstream(mActiveFilepath);

            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(filepath);
            UpdateWindowTitle(projectName);
        }
    }

    void EditorLayer::OpenProject()
    {
        const char* filepath = OS::SelectFolder("Select a project folder to open");
        if (filepath)
        {
            Vector<String> paths = OS::GetAllDirsInPath(filepath);
            Uint sceneCounter = 0;
            String luckyScenePath = "";
            for (auto& path : paths)
            {
                if (OS::GetExtension(path.c_str()) == ".electro")
                {
                    sceneCounter++;
                    if (luckyScenePath.empty())
                        luckyScenePath = path;
                }
            }
            if (sceneCounter == 0 || luckyScenePath.empty())
            {
                ELECTRO_WARN("No file with extension '.electro' found in folder, Invalid Electro Project!");
                ELECTRO_WARN("All ElectroProjects have a '.electro' scene file in the root path, the selected folder doesn't!");
                return;
            }

            InitSceneEssentials();
            Vault::Init(filepath);

            SceneSerializer serializer(mEditorScene, this);
            serializer.Deserialize(luckyScenePath);

            String projectName = OS::GetNameWithoutExtension(filepath);
            UpdateWindowTitle(projectName);
        }
    }

    void EditorLayer::OpenScene()
    {
        auto filepath = OS::OpenFile("ElectroFile (*.electro)\0*.electro;");
        if (filepath)
        {
            mActiveFilepath = *filepath;
            InitSceneEssentials();

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
            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(*filepath);
            ELECTRO_INFO("Scene serialized succesfully!");
        }
    }

    void EditorLayer::SaveScene()
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

    void EditorLayer::InitSceneEssentials()
    {
        mEditorScene.Reset();
        mEditorScene = Ref<Scene>::Create();
        mEditorScene->OnViewportResize((Uint)mViewportSize.x, (Uint)mViewportSize.y);
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);
        mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
        mSceneHierarchyPanel.SetContext(mEditorScene);
    }
}
