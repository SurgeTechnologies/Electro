//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "AssetExtensions.hpp"
#include <yaml-cpp/yaml.h>
#include "imgui.h"

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
        else
            return AssetType::NONE;
    }

    String AssetManager::GetAbsolutePath(const AssetMetadata& metadata)
    {
        String result = fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), metadata.Path.string());
        return result;
    }

    String AssetManager::GetRelativePath(const String& absolutePath)
    {
        String result = std::filesystem::relative(absolutePath, ProjectManager::GetAssetsDirectory()).string();
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

    bool AssetManager::ReloadData(AssetHandle assetHandle)
    {
        auto& metadata = GetMetadata(assetHandle);
        if (!metadata.IsDataLoaded)
        {
            Log::Warn("Trying to Reload Asset that was never loaded");

            Ref<Asset> asset;
            metadata.IsDataLoaded = AssetLoader::TryLoadData(metadata, asset);
            return metadata.IsDataLoaded;
        }

        E_ASSERT(sLoadedAssets.find(assetHandle) != sLoadedAssets.end(), "Asset is not in Loaded asset cache, but its Data is already loaded!");

        Ref<Asset>& asset = sLoadedAssets.at(assetHandle);
        metadata.IsDataLoaded = AssetLoader::TryLoadData(metadata, asset);
        return metadata.IsDataLoaded;
    }

    AssetHandle AssetManager::ExistsInRegistry(const String& absPath)
    {
        if (sAssetRegistry.Contains(absPath))
            return sAssetRegistry[absPath].Handle;

        return INVALID_ASSET_HANDLE;
    }

    void AssetManager::OnImGuiRender(bool* open)
    {
        ImGui::Begin("Asset Manager", open);
        if (ImGui::CollapsingHeader("Loaded Assets"))
        {
            for (const auto& [handle, asset] : sLoadedAssets)
            {
                AssetHandle assetHandle = asset->GetHandle();
                if (handle == assetHandle)
                {
                    AssetMetadata metadata = GetMetadata(assetHandle);
                    ImGui::Text("UUID: %llu", assetHandle);
                    ImGui::Text("Path: %s", metadata.Path.string().c_str());
                    ImGui::Text("Type: %s", Utils::AssetTypeToString(asset->GetType()));
                    ImGui::Separator();
                }
                else
                    Log::Error("Invalid asset handle - {0}!", handle);
            }
        }
        if (ImGui::CollapsingHeader("Assets Registry"))
        {
            for (const auto& [path, metadata] : sAssetRegistry)
            {
                ImGui::Text("UUID: %llu", metadata.Handle);
                ImGui::Text("Path: %s", metadata.Path.string().c_str());
                ImGui::Text("Type: %s", Utils::AssetTypeToString(metadata.Type));
                ImGui::Separator();
            }
        }
        ImGui::End();
    }
}
