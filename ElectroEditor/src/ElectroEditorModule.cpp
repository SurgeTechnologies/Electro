//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include <Electro.hpp>
#include "ElectroEditorModule.hpp"
#include "Asset/ElectroAssetManager.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroSceneRenderer.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "Math/ElectroMath.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroUIMacros.hpp"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

namespace Electro
{
    EditorModule::EditorModule()
        : mVaultPanel(this)
    {
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Resources/ThirdParty/physx.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/Prototype.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/Folder.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/CSharpIcon.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/ElectroIcon.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/UnknownIcon.png"));
        AssetManager::Submit<Texture2D>(EGenerator::CreateTexture2D("Electro/assets/textures/3DFileIcon.png"));
        mPhysicsSettingsPanel.Init();
        mVaultPanel.Init();
        mSceneHierarchyPanel.Init();
        mMaterialPanel.Init();
    }

    void EditorModule::Init()
    {
        Application::Get().GetWindow().RegisterEditorModule(this);
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.SwapChainTarget = false;
        fbSpec.Name = "EditorModuleFramebuffer";
        mFramebuffer = EGenerator::CreateFramebuffer(fbSpec);
        AssetManager::Submit<Framebuffer>(mFramebuffer);

        mEditorScene = Ref<Scene>::Create();
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);
        mSceneHierarchyPanel.SetContext(mEditorScene);
        UpdateWindowTitle("<Null Project>");
        ScriptEngine::SetSceneContext(mEditorScene);
        ImGui::SetWindowFocus(VIEWPORT_TITLE);
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
            case EditorModule::SceneState::Edit:
                mEditorCamera.OnUpdate(ts);
                mEditorScene->OnUpdateEditor(ts, mEditorCamera); break;

            case EditorModule::SceneState::Play:
                if (mViewportFocused) mEditorCamera.OnUpdate(ts);
                mRuntimeScene->OnUpdate(ts);
                mRuntimeScene->OnUpdateRuntime(ts); break;

            case EditorModule::SceneState::Pause:
                if (mViewportFocused) mEditorCamera.OnUpdate(ts);
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
                UI::ToolTip("Don't press this!");
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
            DrawRectAroundWindow({ 1.0f, 1.0f, 0.0f, 1.0f });
        else if (mSceneState == SceneState::Pause)
            DrawRectAroundWindow({ 0.0f, 0.0f, 1.0f, 1.0f });

        mViewportFocused = ImGui::IsWindowFocused();
        mViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiModule()->BlockEvents(!mViewportFocused || !mViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        UI::Image(mFramebuffer->GetColorAttachmentID(0), mViewportSize);
        {
            auto data = UI::DragAndDropTarget(ELECTRO_SCENE_FILE_DND_ID);
            if (data)
            {
                InitSceneEssentials();
                SceneSerializer serializer(mEditorScene, this);
                String filepath = *(String*)data->Data;
                serializer.Deserialize(filepath);
                mActiveFilepath = filepath;
            }
        }
        {
            auto data = UI::DragAndDropTarget(MESH_DND_ID);
            if (data)
                mEditorScene->CreateEntity("Mesh").AddComponent<MeshComponent>().Mesh = EGenerator::CreateMesh(*(String*)data->Data);
        }
        RenderGizmos();
        UI::EndViewport();

        if (mShowRendererSettingsPanel)
        {
            ImGui::Begin(RENDERER_SETTINGS_TITLE, &mShowRendererSettingsPanel);
            if (ImGui::CollapsingHeader("Environment"))
            {
                ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV;
                ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
                UI::Color4("Clear Color", mClearColor);

                if (ImGui::BeginTable("EnvMapTable", 2, flags))
                {
                    ImGui::TableSetupColumn("##col1", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                    ImGui::TableSetupColumn("##col2", ImGuiTableColumnFlags_WidthFixed, contentRegionAvailable.x * 0.6156f);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Path");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-1);
                    Ref<EnvironmentMap>& environmentMap = SceneRenderer::GetEnvironmentMapSlot();
                    if (environmentMap && !environmentMap->GetPath().empty())
                        ImGui::InputText("##envfilepath", (char*)environmentMap->GetPath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                    else
                        ImGui::InputText("##envfilepath", (char*)"", 256, ImGuiInputTextFlags_ReadOnly);
                    auto dropData = UI::DragAndDropTarget(TEXTURE_DND_ID);
                    if (dropData)
                        environmentMap = EGenerator::CreateEnvironmentMap(*(String*)dropData->Data);
                    ImGui::EndTable();

                    if (ImGui::Button("Open"))
                    {
                        std::optional<String> filepath = OS::OpenFile("*.hdr");
                        if (filepath)
                            environmentMap = EGenerator::CreateEnvironmentMap(*filepath);
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
                        UI::SliderFloat("Skybox LOD", environmentMap->mTextureLOD, 0, 10);
                        UI::SliderFloat("Intensity", environmentMap->mIntensity, 1, 100);
                    }
                }
            }
            if (ImGui::CollapsingHeader("Debug"))
            {
                Pair<bool*, bool*> debugData = RendererDebug::GetToggles();
                UI::Checkbox("Show Grid", debugData.Data1, 160.0f);
                UI::Checkbox("Show Camera Frustum", debugData.Data2, 160.0f);
            }
            ImGui::End();
        }
        RenderPanels();

        if (mShowWelcomePopup)
        {
            ImGui::OpenPopup("Welcome to Electro!");
            mShowWelcomePopup = false;
        }

        ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2{ 400,0 });
        if (ImGui::BeginPopupModal("Welcome to Electro!", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(ICON_ELECTRO_CIRCLE" You can create a project by going to File > New Project");
            ImGui::TextUnformatted(ICON_ELECTRO_CIRCLE" You can open a project(.electro file) by going to File > Open");
            ImGui::Separator();
            if(ImGui::Button(ICON_ELECTRO_GITHUB))
                OS::OpenURL("https://github.com/FahimFuad/Electro");
            UI::ToolTip("Open Electro's github");
            ImGui::SameLine();

            if (ImGui::Button(ICON_ELECTRO_CODE))
                OS::OpenURL("https://github.com/FahimFuad/Electro/blob/master/Resources/Docs/CSharpScriptSystem.md");
            UI::ToolTip("How do I do C# scripting?");
            ImGui::SameLine();

            if (ImGui::Button("OK"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

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

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
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
        }
        return false;
    }

    void EditorModule::UpdateWindowTitle(const String& sceneName)
    {
        auto& app = Application::Get();
        String config = app.GetBuildConfig();
        String title = "Electro - " + sceneName + " - " + config;
        app.GetWindow().SetTitle(title);
    }

    void EditorModule::DrawRectAroundWindow(const glm::vec4& color)
    {
        ImVec2 windowMin = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 windowMax = { windowMin.x + windowSize.x, windowMin.y + windowSize.y };
        ImGui::GetForegroundDrawList()->AddRect(windowMin, windowMax, ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    }

    void EditorModule::RenderGizmos()
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
    }

    void EditorModule::NewProject()
    {
        const char* filepath = OS::SelectFolder("Choose a location to save the project");
        if (filepath)
        {
            InitSceneEssentials();
            mVaultPath = filepath;

            AssetManager::Init(mVaultPath);
            OS::CreateOrEnsureFolderExists(mVaultPath.c_str(), "Scenes");
            String scenePath = mVaultPath + "/" + "Scenes";

            //TODO: Automate this project name
            String projectName = OS::GetNameWithoutExtension(filepath);

            std::ofstream stream = std::ofstream(scenePath + String("/") + projectName + ".electro"); // Create the file
            if (stream.bad())
                ELECTRO_ERROR("Bad stream!");

            SceneSerializer serializer(mEditorScene, this);
            serializer.Serialize(filepath);
            UpdateWindowTitle(projectName);
            mActiveFilepath = scenePath + String("/") + projectName + ".electro";
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

            AssetManager::Init(mVaultPath);
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
        mEditorScene->OnViewportResize((Uint)mViewportSize.x, (Uint)mViewportSize.y);
        mEditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);
        mEditorCamera.SetViewportSize(mViewportSize.x, mViewportSize.y);
        mSceneHierarchyPanel.SetContext(mEditorScene);
    }
}
