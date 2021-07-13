//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "AssetImporter.hpp"
#include "Asset/AssetManager.hpp"
#include "TextureImporter.hpp"

namespace Electro
{
    std::unordered_map<AssetType, Scope<IAssetImporter>> AssetImporter::sImporters;

    void AssetImporter::Init()
    {
        sImporters[AssetType::TEXTURE2D] = CreateScope<TextureImporter>();
    }

    void AssetImporter::Shutdown()
    {
        sImporters.clear();
    }

    bool AssetImporter::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset)
    {
        if (sImporters.find(metadata.Type) == sImporters.end())
        {
            Log::Warn("There is currently no importer for assets of type {0}", metadata.Path.extension().string());
            return false;
        }

        return sImporters[metadata.Type]->LoadData(metadata, asset);
    }

    bool AssetImporter::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset)
    {
        if (sImporters.find(metadata.Type) == sImporters.end())
        {
            Log::Warn("There is currently no serializer for assets of type {0}", metadata.Path.extension().string());
            return false;
        }

        return sImporters[metadata.Type]->SaveData(metadata, asset);
    }
}