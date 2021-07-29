//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "AssetExtensions.hpp"
#include <yaml-cpp/yaml.h>

namespace Electro
{
    void AssetManager::Init()
    {
        AssetLoader::Init();
        DeserializeRegistry();
    }

    void AssetManager::Shutdown()
    {
        SerializeRegistry();
        sLoadedAssets.clear();
        sAssetRegistry.Clear();
        AssetLoader::Shutdown();
    }

    static AssetMetadata sNullMetadata;
    void AssetManager::SerializeRegistry()
    {
        // Sort assets by UUID - then serialize
        struct AssetRegistryEntry
        {
            String FilePath;
            AssetType Type;
        };

        std::map<AssetHandle, AssetRegistryEntry> sortedMap;
        for (auto& [filepath, metadata] : sAssetRegistry)
        {
            String pathToSerialize = metadata.Path.string();
            std::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
            sortedMap[metadata.Handle] = { pathToSerialize, metadata.Type };
        }

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Assets" << YAML::BeginSeq;
        for (auto& [handle, entry] : sortedMap)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
            out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(entry.Type);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        const String& assetRegistryPath = ProjectManager::GetAssetRegistryCachePath().string();
        FileSystem::WriteFile(assetRegistryPath, out.c_str());
    }

    void AssetManager::DeserializeRegistry()
    {
        const String& assetRegistryPath = ProjectManager::GetAssetRegistryCachePath().string();

        if (!FileSystem::Exists(assetRegistryPath))
            return;

        YAML::Node data = YAML::LoadFile(assetRegistryPath);
        YAML::Node handles = data["Assets"];
        if (!handles)
        {
            Log::Error("Invalid AssetRegistry File!");
            return;
        }

        sAssetRegistry.Clear();

        for (const auto& entry : handles)
        {
            String filepath = entry["FilePath"].as<String>();

            AssetMetadata metadata;
            metadata.Handle = entry["Handle"].as<uint64_t>();
            metadata.Path   = filepath; //! filepath is already relative, as we load it from the disk
            metadata.Type   = static_cast<AssetType>(Utils::AssetTypeFromString(entry["Type"].as<String>()));

            if (metadata.Type == AssetType::NONE)
                continue;

            if (!FileSystem::Exists(AssetManager::GetAbsolutePath(metadata)))
            {
                Log::Warn("Missing asset '{0}' found in AssetRegistry file!", metadata.Path.string());
                //TODO: Maybe try to relocate the missing asset here?
                continue;
            }

            if (metadata.Handle == 0)
            {
                Log::Warn("INVALID AssetHandle found for {0}", metadata.Path.string());
                continue;
            }

            sAssetRegistry[metadata.Path] = metadata;
        }
    }

    AssetType AssetManager::GetAssetTypeFromExtension(const String& extension)
    {
        if (sAssetExtensionMap.find(extension) != sAssetExtensionMap.end())
            return sAssetExtensionMap.at(extension);

        return AssetType::NONE;
    }

    String AssetManager::GetAbsolutePath(const AssetMetadata& metadata)
    {
        String result = fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), metadata.Path.string());
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }

    String AssetManager::GetAbsolutePath(const String& path)
    {
        String result = fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), path);
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }

    String AssetManager::GetRelativePath(const String& absolutePath)
    {
        String result = std::filesystem::relative(absolutePath, ProjectManager::GetAssetsDirectory()).string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }

    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::sLoadedAssets;
    AssetRegistry AssetManager::sAssetRegistry;

    AssetHandle AssetManager::ImportAsset(const String& filepath)
    {
        std::filesystem::path path = std::filesystem::relative(filepath, ProjectManager::GetAssetsDirectory());

        // Already in the registry
        if (sAssetRegistry.Contains(path))
            return INVALID_ASSET_HANDLE;

        AssetType type = GetAssetTypeFromExtension(FileSystem::GetExtension(path.string()));
        if (type == AssetType::NONE)
            return INVALID_ASSET_HANDLE;

        AssetMetadata metadata;
        metadata.Handle = AssetHandle();
        metadata.Path = path; // Relative path is stored
        metadata.Type = type;
        metadata.IsDataLoaded = false;

        // We don't load the data, just import the asset, save to the registry
        sAssetRegistry[metadata.Path] = metadata;
        return metadata.Handle;
    }

    AssetMetadata& AssetManager::GetMetadata(AssetHandle handle)
    {
        for (auto& [filepath, metadata] : sAssetRegistry)
        {
            if (metadata.Handle == handle)
                return metadata;
        }

        return sNullMetadata;
    }

    void AssetManager::RemoveAsset(AssetHandle assetHandle)
    {
        // Check if the given handle exists in the Registry or not
        bool found = false;
        for (const auto& assetRegistryItem : sAssetRegistry)
        {
            if (assetRegistryItem.second.Handle == assetHandle)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;

        AssetMetadata metadata = GetMetadata(assetHandle);
        sAssetRegistry.Remove(metadata.Path);

        if (sLoadedAssets.find(assetHandle) != sLoadedAssets.end())
            sLoadedAssets.erase(assetHandle);
    }

    AssetHandle AssetManager::GetHandleFromPath(const String& assetPath)
    {
        std::filesystem::path path = assetPath;
        if (sAssetRegistry.Contains(path))
            return sAssetRegistry.Get(path).Handle;

        return INVALID_ASSET_HANDLE;
    }

    bool AssetManager::ExistsInRegistry(const String& absPath)
    {
        return sAssetRegistry.Contains(absPath);
    }

    bool AssetManager::IsLoaded(const AssetHandle& handle)
    {
        if (sLoadedAssets.find(handle) != sLoadedAssets.end())
            return true;

        return false;
    }
}
