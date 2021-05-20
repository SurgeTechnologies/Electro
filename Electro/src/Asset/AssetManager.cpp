//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Factory.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include <yaml-cpp/yaml.h>

namespace Electro
{
    String AssetManager::sProjectPath = String();
    bool AssetManager::sAssetManagerInitialized = false;

    AssetRegistry<Shader> AssetManager::sShaderRegistry;
    AssetRegistry<Texture2D> AssetManager::sTexture2DRegistry;
    AssetRegistry<EnvironmentMap> AssetManager::sEnvMapRegistry;
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
        sAssetManagerInitialized = false;
    }

    //TODO: Rework this function
    void AssetManager::RemoveIfExists(const String& path)
    {
        if (Exists<Shader>(path))
        {
            AssetRegistry<Shader>& registry = GetRegistry<Shader>();
            Remove<Shader>(path); return;
        }
        else if (Exists<Texture2D>(path))
        {
            AssetRegistry<Texture2D>& registry = GetRegistry<Texture2D>();
            Remove<Texture2D>(path); return;
        }
        else if (Exists<EnvironmentMap>(path))
        {
            AssetRegistry<EnvironmentMap>& registry = GetRegistry<EnvironmentMap>();
            Remove<EnvironmentMap>(path); return;
        }
        else if (Exists<PhysicsMaterial>(path))
        {
            AssetRegistry<PhysicsMaterial>& registry = GetRegistry<PhysicsMaterial>();
            Remove<PhysicsMaterial>(path); return;
        }
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }

    void AssetSerializer::SerializePhysicsMaterial(const String& path, Ref<PhysicsMaterial>& pmat)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "StaticFriction" << YAML::Value << pmat->mStaticFriction;
        out << YAML::Key << "DynamicFriction" << YAML::Value << pmat->mDynamicFriction;
        out << YAML::Key << "Bounciness" << YAML::Value << pmat->mBounciness;
        out << YAML::EndMap;

        FileSystem::WriteFile(path, out.c_str());
    }

    Ref<PhysicsMaterial> AssetSerializer::DeserializePhysicsMaterial(const String& path)
    {
        std::ifstream stream(path);
        if (!stream.is_open())
        {
            ELECTRO_ERROR("AssetSerializer: Cannot open filepath %s", path.c_str());
            return nullptr;
        }

        YAML::Node data;
        try { data = YAML::LoadFile(path); }
        catch (const YAML::ParserException& ex) { ELECTRO_ERROR("Failed to load file '%s'\n %s", path.c_str(), ex.what()); }

        Ref<PhysicsMaterial> asset = Factory::CreatePhysicsMaterial(path);
        asset->mStaticFriction = data["StaticFriction"].as<float>();
        asset->mDynamicFriction = data["DynamicFriction"].as<float>();
        asset->mBounciness = data["Bounciness"].as<float>();
        return asset;
    }

}
