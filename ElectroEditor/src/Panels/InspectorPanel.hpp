//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Scene/Entity.hpp"
#include "AssetEditors/IAssetEditor.hpp"

namespace Electro
{
    class SceneHierarchyPanel;
    class AssetsPanel;
    class InspectorPanel : public IPanel
    {
    public:
        virtual void OnInit(void* data = nullptr) override;
        virtual void OnImGuiRender(bool* show) override;

        template <typename T>
        void Show(AssetHandle handle, AssetType assetType)
        {
            mSceneHierarchy->ClearSelectedEntity();
            mAssetEditors[assetType]->SetForEdit(AssetManager::GetAsset<T>(handle).As<Asset>());
        }
    private:
        void DrawComponents(Entity entity);
    private:
        SceneHierarchyPanel* mSceneHierarchy;
        AssetsPanel* mAsssetsPanel;

        std::unordered_map<AssetType, Scope<IAssetEditor>> mAssetEditors;
    };
}