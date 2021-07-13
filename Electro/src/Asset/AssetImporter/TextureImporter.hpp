//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IAssetImporter.hpp"

namespace Electro
{
    class TextureImporter : public IAssetImporter
    {
    public:
        virtual bool LoadData(AssetMetadata metaData, Ref<Asset>& asset) override;
        virtual bool SaveData(AssetMetadata metaData, const Ref<Asset>& asset) override;
    };
}
