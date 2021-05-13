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

    AssetHandle AssetManager::GetHandle(const String& nameWithExtension)
    {
        for (const auto& [handle, asset] : sRegistry)
            if (asset->mName == nameWithExtension)
                return handle;

        AssetHandle nullHandle;
        nullHandle.MakeInvalid();
        return nullHandle;
    }

    void AssetManager::Remove(const AssetHandle& assetHandle)
    {
        E_ASSERT(assetHandle.IsValid(), "Invalid asset Handle!");
        sRegistry.erase(assetHandle);
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
