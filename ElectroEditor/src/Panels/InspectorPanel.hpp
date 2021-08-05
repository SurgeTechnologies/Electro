//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Scene/Entity.hpp"
#include "AssetEditors/MaterialEditor.hpp"

namespace Electro
{
    class SceneHierarchyPanel;
    class AssetsPanel;
    class MaterialEditor;
    class InspectorPanel : public IPanel
    {
    public:
        virtual void OnInit(void* data = nullptr) override;
        virtual void OnImGuiRender(bool* show) override;
        template <typename T>
        void Show(AssetHandle handle) 
        {
            if constexpr (std::is_same_v<T, Material>)
            {
                mSceneHierarchy->ClearSelectedEntity();
                mMaterialEditor.SetMaterial(AssetManager::GetAsset<Material>(handle));
            }
        }
    private:
        void DrawComponents(Entity entity);
    private:
        SceneHierarchyPanel* mSceneHierarchy;
        AssetsPanel* mAsssetsPanel;
        MaterialEditor mMaterialEditor;
    };
}