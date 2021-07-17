//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IAssetLoader.hpp"

namespace Electro
{
    class TextureLoader : public IAssetLoader
    {
    public:
        virtual bool LoadData(AssetMetadata metaData, Ref<Asset>& asset) override;
        virtual bool SaveData(AssetMetadata metaData, const Ref<Asset>& asset) override;
    };

    class EnvMapLoader : public IAssetLoader
    {
    public:
        virtual bool LoadData(AssetMetadata metaData, Ref<Asset>& asset) override;
        virtual bool SaveData(AssetMetadata metaData, const Ref<Asset>& asset) override;
    };
}
