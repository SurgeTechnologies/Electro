//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Factory.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Texture.hpp"

namespace Electro
{
    String AssetManager::sProjectPath = String();
    bool AssetManager::sAssetManagerInitialized = false;

    AssetRegistry<Shader> AssetManager::sShaderRegistry;
    AssetRegistry<Texture2D> AssetManager::sTexture2DRegistry;
    AssetRegistry<EnvironmentMap> AssetManager::sEnvMapRegistry;
    AssetRegistry<Material> AssetManager::sMaterialRegistry;
    AssetRegistry<PhysicsMaterial> AssetManager::sPhysicsMaterialRegistry;

    void AssetManager::Init(const String& projectPath)
    {
        sProjectPath = projectPath;
        sAssetManagerInitialized = true;
    }

    void AssetManager::Shutdown()
    {
        sProjectPath.clear();
        sShaderRegistry.clear();
        sTexture2DRegistry.clear();
        sEnvMapRegistry.clear();
        sMaterialRegistry.clear();
        sPhysicsMaterialRegistry.clear();
        sAssetManagerInitialized = false;
    }

    //TODO: Rework this function
    void AssetManager::RemoveIfExists(const String& path)
    {
        if (Exists<Shader>(path))
            Remove<Shader>(path);

        else if (Exists<Texture2D>(path))
            Remove<Texture2D>(path);

        else if (Exists<EnvironmentMap>(path))
            Remove<EnvironmentMap>(path);

        else if (Exists<Material>(path))
            Remove<Material>(path);

        else if (Exists<PhysicsMaterial>(path))
            Remove<PhysicsMaterial>(path);
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
