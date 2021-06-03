//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Module.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Interface/Framebuffer.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ProfilerPanel.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/MaterialPanel.hpp"
#include "Panels/PhysicsSettingsPanel.hpp"
#include "Panels/CodeEditorPanel.hpp"

namespace Electro
{
    class EditorModule : public Module
    {
    public:
        EditorModule();
        virtual ~EditorModule() = default;

        virtual void Init() override;
        virtual void Shutdown() override;

        void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        void OnEvent(Event& e) override;
        const Ref<Framebuffer>& GetFramebuffer() const { return mFramebuffer; }

        void NewProject();
        void Open();
        void SaveScene();
        void SaveSceneAs();
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void UpdateWindowTitle(const String& sceneName);
        void RenderGizmos();
        void RenderPanels();
        void InitSceneEssentials();
        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();
        void OnSceneResume();
    public:
        bool mShowHierarchyAndInspectorPanel = true;
        bool mShowConsolePanel = true;
        bool mShowVaultAndCachePanel = true;
        bool mShowMaterialPanel = true;
        bool mShowRendererSettingsPanel = false;
        bool mShowProfilerPanel = false;
        bool mShowPhysicsSettingsPanel = false;
        bool mShowCodeEditorPanel = false;
    private:
        enum class SceneState
        {
            Edit = 0, Play = 1, Pause = 2
        };
        SceneState mSceneState = SceneState::Edit;

        Ref<Framebuffer> mFramebuffer;
        Ref<Scene> mEditorScene, mRuntimeScene;
        EditorCamera mEditorCamera;
        Entity mSelectedEntity;

        bool mViewportFocused = false, mViewportHovered = false;
        glm::vec4 mClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        glm::vec2 mViewportSize = { 0.0f, 0.0f };
        glm::vec2 mViewportBounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
        String mActiveFilepath = String();
        String mAssetsPath = String();
        int mGizmoType = -1;
        bool mGizmoInUse = false;

        SceneHierarchyPanel mSceneHierarchyPanel;
        ProfilerPanel mProfilerPanel;
        AssetsPanel mVaultPanel;
        MaterialPanel mMaterialPanel;
        PhysicsSettingsPanel mPhysicsSettingsPanel;
        CodeEditorPanel mCodeEditorPanel;
    private:
        friend class AssetsPanel;
        friend class SceneSerializer;
    };
}
