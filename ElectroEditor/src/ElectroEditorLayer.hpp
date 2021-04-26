//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroLayer.hpp"
#include "Renderer/Camera/ElectroEditorCamera.hpp"
#include "Renderer/Interface/ElectroFramebuffer.hpp"
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
        Ref<Framebuffer> GetFramebuffer() { return mFramebuffer; }

        void NewProject();
        void OpenProject();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void UpdateWindowTitle(const String& sceneName);
        void DrawRectAroundWindow(const glm::vec4& color);
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
        glm::vec2 mViewportSize = { 0.0f, 0.0f };
        glm::vec2 mViewportBounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
        String mActiveFilepath = String();
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
