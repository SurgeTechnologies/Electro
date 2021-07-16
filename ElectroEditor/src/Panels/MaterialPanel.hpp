//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Scene/Entity.hpp"
#include "SceneHierarchyPanel.hpp"

namespace Electro
{
    class MaterialPanel : public IPanel
    {
    public:
        MaterialPanel() = default;
        ~MaterialPanel() = default;
        virtual void OnInit(void* hierarchy) override;
        virtual void OnImGuiRender(bool* show) override;
    private:
        SceneHierarchyPanel* mSceneHierarchyPanel;
    };
}
