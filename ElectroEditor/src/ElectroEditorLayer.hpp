//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <Electro.hpp>
#include "Renderer/ElectroEditorCamera.hpp"
#include "Panels/ElectroConsolePanel.hpp"
#include "Panels/ElectroSceneHierarchyPanel.hpp"
#include "Panels/ElectroProfilerPanel.hpp"
#include "Panels/ElectroVaultPanel.hpp"
#include "Panels/ElectroMaterialPanel.hpp"
#include "Panels/ElectroPhysicsSettingsPanel.hpp"

namespace Electro
{
    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        void OnEvent(Event& e) override;
    private:
        bool OnKeyPressed(KeyPressedEvent& e);

        void NewScene();
        void OpenFolder();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
        void UpdateWindowTitle(const String& sceneName);
        void DrawRectAroundWindow(const glm::vec4& color);
        void RenderGizmos();
        void RenderPanels();

        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();
        void OnSceneResume();
    private:
        enum class SceneState
        {
            Edit = 0, Play = 1, Pause = 2
        };
        SceneState mSceneState = SceneState::Edit;

        Ref<Framebuffer> mFramebuffer;
        Ref<Scene> mEditorScene, mRuntimeScene;
        EditorCamera mEditorCamera;
        Entity mStartupCameraEntity;
        Entity mSelectedEntity;

        bool mViewportFocused = false, mViewportHovered = false;
        glm::vec4 mClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec2 mViewportBounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
        bool mFirstTimeSave = false;
        String mActiveFilepath = String();
        String mCurrentSkyboxPath = String();
        int mGizmoType = -1;
        bool mGizmoInUse = false;

        SceneHierarchyPanel mSceneHierarchyPanel;
        ProfilerPanel mProfilerPanel;
        VaultPanel mVaultPanel;
        MaterialPanel mMaterialPanel;
        PhysicsSettingsPanel mPhysicsSettingsPanel;
    private:
        friend class VaultPanel;
        friend class SceneSerializer;
    };
}
