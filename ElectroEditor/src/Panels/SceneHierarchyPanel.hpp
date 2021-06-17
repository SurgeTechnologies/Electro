//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Events/KeyEvent.hpp"
#include "Scene/Entity.hpp"
#include "IPanel.hpp"

namespace Electro
{
    class SceneHierarchyPanel : public IPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& context);
        virtual void Init(void* data) override;
        virtual void OnImGuiRender(bool* show) override;

        void SetContext(const Ref<Scene>& context);
        void ClearSelectedEntity() { mSelectionContext = {}; }

        E_FORCE_INLINE Entity& GetSelectedEntity() { return mSelectionContext; };
        void SetSelectedEntity(Entity entity) { mSelectionContext = entity; }
        Ref<Scene> GetCurrentScene() const { return mContext; };

        void OnEvent(Event& e);
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

        Ref<Scene> mContext;
    private:
        bool mIsHierarchyFocused = false;
        bool mIsHierarchyHovered = false;
        Entity mSelectionContext;
    };
}
