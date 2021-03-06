//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Asset/AssetMetadata.hpp"

namespace Electro
{
    class IAssetLoader
    {
    public:
        virtual ~IAssetLoader() = default;

        virtual bool LoadData(AssetMetadata metaData, Ref<Asset>& asset) = 0;
        virtual bool SaveData(AssetMetadata metaData, const Ref<Asset>& asset) = 0;
    };
}