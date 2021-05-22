//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Scene/Entity.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/KeyEvent.hpp"

namespace Electro
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& context);
        void Init();
        void SetContext(const Ref<Scene>& context);
        void ClearSelectedEntity() { mSelectionContext = {}; }

        E_FORCE_INLINE Entity& GetSelectedEntity() { return mSelectionContext; };
        void SetSelectedEntity(Entity entity) { mSelectionContext = entity; }
        Ref<Scene> GetCurrentScene() const { return mContext; };

        void OnImGuiRender(bool* show);
        void OnEvent(Event& e);
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

        Ref<Scene> mContext;
    private:
        RendererID mPrototypeTextureID;
        bool mIsHierarchyFocused = false;
        bool mIsHierarchyHovered = false;
        Entity mSelectionContext;
    };
}
