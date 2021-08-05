//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "TextureLoader.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    bool TextureLoader::LoadData(AssetMetadata metaData, Ref<Asset>& asset)
    {
        Texture2DSpecification spec;
        spec.GenerateMips = true;
        spec.SRGB = false;
        spec.Path = (ProjectManager::GetAssetsDirectory() / metaData.Path).string();
        Ref<Texture2D> result = Texture2D::Create(spec);

        if (result->Loaded())
        {
            asset = result;
            asset->SetHandle(metaData.Handle);
            asset->SetType(metaData.Type);
            asset->SetFlag(AssetFlag::VALID);
            return true;
        }

        asset->SetFlag(AssetFlag::INVALID);
        return false;
    }

    bool TextureLoader::SaveData(AssetMetadata metaData, const Ref<Asset>& asset)
    {
        // TODO Implement
        E_INTERNAL_ASSERT("Not Implemented!");
        return false;
    }

    bool EnvMapLoader::LoadData(AssetMetadata metaData, Ref<Asset>& asset)
    {
        Ref<EnvironmentMap> result = EnvironmentMap::Create((ProjectManager::GetAssetsDirectory() / metaData.Path).string());

        if (result)
        {
            asset = result;
            asset->SetHandle(metaData.Handle);
            asset->SetType(metaData.Type);
            asset->SetFlag(AssetFlag::VALID);
            return true;
        }

        asset->SetFlag(AssetFlag::INVALID);
        return false;
    }

    bool EnvMapLoader::SaveData(AssetMetadata metaData, const Ref<Asset>& asset)
    {
        // TODO Implement
        E_INTERNAL_ASSERT("Not Implemented!");
        return false;
    }
}
