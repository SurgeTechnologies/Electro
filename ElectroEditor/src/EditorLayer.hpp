//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Layer.hpp"
#include "Project/Project.hpp"
#include "Renderer/Camera/EditorCamera.hpp"

// Panels
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/ProfilerPanel.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/PhysicsSettingsPanel.hpp"
#include "Panels/RendererSettingsPanel.hpp"
#include "Panels/ProjectSettingsPanel.hpp"
#include "Panels/AssetRegistryPanel.hpp"
#include "PanelManager.hpp"

namespace Electro
{
    class EditorLayer final : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() override = default;

        virtual void Init() override;
        virtual void Shutdown() override {}

        void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        void OnEvent(Event& e) override;

        void NewProject();
        void OpenProject();
        void SaveScene();
        void SaveSceneAs();
        void ExportProject();
        void SerializeScene(const String& path);
        void DeserializeScene(const String& path);
        void InitSceneEssentials(const String& sceneName);
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void UpdateWindowTitle(const String& projectName);
        void RenderGizmos();
        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();
        void OnSceneResume();
    public:
        bool mShowConsole = true;
        bool mShowHierarchy = true;
        bool mShowInspector = true;
        bool mShowAssetsPanel = true;
        bool mShowRendererSettingsPanel = true;
        bool mShowProfilerPanel = true;
        bool mShowPhysicsSettingsPanel = false;
        bool mShowProjectSettingsPanel = false;
        bool mShowAssetRegistryPanel = false;
    private:
        enum class SceneState { Edit = 0, Play = 1, Pause = 2 };
        SceneState mSceneState = SceneState::Edit;

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
        bool mOpenExportProjectWindow = false;

        char mInputBuffer[INPUT_BUFFER_LENGTH];
        char mNameBuffer[INPUT_BUFFER_LENGTH];
        char mSceneNameBuffer[INPUT_BUFFER_LENGTH];

        SceneHierarchyPanel mSceneHierarchyPanel;
        InspectorPanel mInspectorPanel;
        ProfilerPanel mProfilerPanel;
        AssetsPanel mAssetsPanel;
        PhysicsSettingsPanel mPhysicsSettingsPanel;
        RendererSettingsPanel mRendererSettingsPanel;
        ProjectSettingsPanel mProjectSettingsPanel;
        AssetRegistryPanel mAssetRegistryPanel;
    private:
        friend class AssetsPanel;
        friend class SceneSerializer;
    };
}
