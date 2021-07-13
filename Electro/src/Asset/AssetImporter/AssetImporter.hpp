//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Asset/AssetBase.hpp"
#include "IAssetImporter.hpp"
#include <unordered_map>

namespace Electro
{
    class AssetImporter
    {
    public:
        static void Init();
        static void Shutdown();

        static bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset);
        static bool Serialize(const AssetMetadata& metadata, Ref<Asset>& asset);
    private:
        static std::unordered_map<AssetType, Scope<IAssetImporter>> sImporters;
    };
}