//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/EnvironmentMap.hpp"

namespace Electro
{
    String AssetManager::sProjectPath = String();
    bool AssetManager::sAssetManagerInitialized = false;
    std::unordered_map<Electro::AssetHandle, Electro::Ref<Electro::Asset>> AssetManager::sRegistry;

    void AssetManager::Init(const String& projectPath)
    {
        sProjectPath = projectPath;
        sAssetManagerInitialized = true;
    }

    void AssetManager::Shutdown()
    {
        sProjectPath.clear();
        sAssetManagerInitialized = false;
    }

    bool AssetManager::Exists(const String& path)
    {
        const AssetHandle handle = GetHandle(path);
        if (handle.IsValid())
        {
            if (sRegistry.find(handle) == sRegistry.end())
                return false; //Asset is not in registry
            else
                return true;
        }
        return false;
    }

    const AssetHandle AssetManager::GetHandle(const String& path)
    {
        for (const auto& [handle, asset] : sRegistry)
            if (asset->mPathInDisk == path)
                return handle;

        AssetHandle nullHandle;
        nullHandle.MakeInvalid();
        return nullHandle;
    }

    bool AssetManager::Remove(const String& path)
    {
        const AssetHandle handle = GetHandle(path);
        return Remove(handle);
    }

    bool AssetManager::Remove(const AssetHandle& assetHandle)
    {
        if (assetHandle.IsValid())
        {
            sRegistry.erase(assetHandle);
            return true;
        }
        return false;
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
