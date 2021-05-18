//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Factory.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/Framebuffer.hpp"

namespace Electro
{
    String AssetManager::sProjectPath = String();
    bool AssetManager::sAssetManagerInitialized = false;
    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::sRegistry;

    void AssetManager::Init(const String& projectPath)
    {
        sProjectPath = projectPath;
        sAssetManagerInitialized = true;
    }

    void AssetManager::Shutdown()
    {
        sProjectPath.clear();
        sRegistry.clear();
        sAssetManagerInitialized = false;
    }

    bool AssetManager::Exists(const String& path)
    {
        AssetHandle handle = GetHandle(path);
        if (handle.IsValid())
        {
            if (sRegistry.find(handle) == sRegistry.end())
                return false; //Asset is not in registry
            else
                return true;
        }
        return false;
    }

    AssetHandle AssetManager::GetHandle(const String& path)
    {
        for (const auto& [handle, asset] : sRegistry)
            if (asset->mPathInDisk == path)
                return handle;

        AssetHandle nullHandle;
        nullHandle.MakeInvalid();
        return nullHandle;
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

    bool AssetManager::Remove(const String& path)
    {
        AssetHandle handle = GetHandle(path);
        return Remove(handle);
    }

    void AssetManager::RemoveIfExists(const String& path)
    {
        if (!AssetManager::Exists(path))
            return;

        ELECTRO_INFO("Previous asset cache size - %i", sRegistry.size());
        if (Remove(path))
            ELECTRO_DEBUG("File - %s was successfully deleted from registry!", FileSystem::GetNameWithExtension(path).c_str());
        ELECTRO_INFO("Current asset cache size - %i", sRegistry.size());
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
