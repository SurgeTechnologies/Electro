//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Module.hpp"
#include "Project/Project.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Interface/Framebuffer.hpp"

// Panels
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ProfilerPanel.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/MaterialPanel.hpp"
#include "Panels/PhysicsSettingsPanel.hpp"
#include "Panels/RendererSettingsPanel.hpp"
#include "Panels/ProjectSettingsPanel.hpp"
#include "PanelManager.hpp"

namespace Electro
{
    class EditorModule final : public Module
    {
    public:
        EditorModule();
        virtual ~EditorModule() override = default;

        virtual void Init() override;
        virtual void Shutdown() override {}

        void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        void OnEvent(Event& e) override;
        [[nodiscard]] const Ref<Framebuffer>& GetFramebuffer() const { return mFramebuffer; }

        void NewProject();
        void OpenProject();
        void SaveScene();
        void SaveSceneAs();
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void UpdateWindowTitle(const String& projectName);
        void RenderGizmos();
        void InitSceneEssentials();
        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();
        void OnSceneResume();

        void SerializeScene(const String& path);
        void DeserializeScene(const String& path);
    public:
        bool mShowHierarchyAndInspectorPanel = true;
        bool mShowAssetsPanel = true;
        bool mShowMaterialPanel = true;
        bool mShowRendererSettingsPanel = false;
        bool mShowProfilerPanel = false;
        bool mShowPhysicsSettingsPanel = false;
        bool mShowProjectSettingsPanel = false;
    private:
        enum class SceneState { Edit = 0, Play = 1, Pause = 2 };
        SceneState mSceneState = SceneState::Edit;

        Ref<Framebuffer> mFramebuffer;
        Ref<Scene> mEditorScene, mRuntimeScene;
        EditorCamera mEditorCamera;
        Entity mSelectedEntity;

        bool mViewportFocused = false, mViewportHovered = false;

        glm::vec4 mClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        glm::vec2 mViewportSize = { 0.0f, 0.0f };
        glm::vec2 mViewportBounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };

        Ref<Project> mActiveProject;
        String mActiveFilepath = String();
        String mActiveSceneName = String();

        int mGizmoType = -1;
        bool mGizmoInUse = false;
        bool mOpenNewProjectWindow = false;

        char mInputBuffer[INPUT_BUFFER_LENGTH];
        char mNameBuffer[INPUT_BUFFER_LENGTH];
        char mSceneNameBuffer[INPUT_BUFFER_LENGTH];

        PanelManager mPanelManager;

        SceneHierarchyPanel mSceneHierarchyPanel;
        ProfilerPanel mProfilerPanel;
        AssetsPanel mAssetsPanel;
        MaterialPanel mMaterialPanel;
        PhysicsSettingsPanel mPhysicsSettingsPanel;
        RendererSettingsPanel mRendererSettingsPanel;
        ProjectSettingsPanel mProjectSettingsPanel;
    private:
        friend class AssetsPanel;
        friend class SceneSerializer;
    };
}
