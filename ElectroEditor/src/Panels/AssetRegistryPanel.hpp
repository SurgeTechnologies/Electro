//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    class AssetRegistryPanel : public IPanel
    {
    public:
        virtual void OnInit(void* data = nullptr) override;
        virtual void OnImGuiRender(bool* show) override;
    private:
        void RenderAssetInfo(const AssetMetadata& metadata, const char* typeString);
        void RenderAssetInfoOnSearch(const AssetMetadata& metadata, const char* typeString, bool isNumber);
    private:
        char mLoadedAssetSearchBuffer[INPUT_BUFFER_LENGTH];
        char mAssetRegistrySearchBuffer[INPUT_BUFFER_LENGTH];

        AssetRegistry* mRegistrySlot = nullptr;
        std::unordered_map<AssetHandle, Ref<Asset>>* mLoadedAssetsSlot = nullptr;
    };
}