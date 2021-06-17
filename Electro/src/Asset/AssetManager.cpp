//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/EnvironmentMap.hpp"

namespace Electro
{
    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::sLoadedAssets;

    void AssetManager::Init()
    {
    }

    void AssetManager::Shutdown()
    {
    }

    bool AssetManager::Exists(const String& path)
    {
        const AssetHandle handle = GetHandle(path);
        if (handle.IsValid())
        {
            if (sLoadedAssets.find(handle) == sLoadedAssets.end())
                return false; //Asset is not in registry
        }
        return false;
    }

    bool AssetManager::Exists(const AssetHandle& handle)
    {
        if (handle.IsValid())
        {
            if (sLoadedAssets.find(handle) == sLoadedAssets.end())
                return false; //Asset is not in registry
            else
                return true;
        }
        return false;
    }

    AssetHandle AssetManager::GetHandle(const String& path)
    {
        for (const auto& [handle, asset] : sLoadedAssets)
        {
            if (asset->GetPath() == path)
                return handle;
        }

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
            sLoadedAssets.erase(assetHandle);
            return true;
        }
        return false;
    }
}
