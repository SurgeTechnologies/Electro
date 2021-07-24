//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/FileSystem.hpp"
#include "AssetRegistry.hpp"
#include "AssetImporter/AssetLoader.hpp"
#include "Project/ProjectManager.hpp"

#include "Renderer/Interface/Texture.hpp"
#include "Renderer/EnvironmentMap.hpp"

namespace Electro
{
    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        // Initializes the AssetManager
        static void Init();

        // Shutdowns the AssetManager, clears all registry
        static void Shutdown();

        // Writes the registry cache to the file
        static void SerializeRegistry();

        // Loads the registry cache from the file
        static void DeserializeRegistry();

        // Returns the Absolute Path from the relative path stored in the metadata
        static String GetAbsolutePath(const AssetMetadata& metadata);

        // Returns the Relative Path from the given absolute path
        static String GetRelativePath(const String& absolutePath);

        // Imports a specific asset, doesn't load the data to the RAM
        static AssetHandle ImportAsset(const String& filepath);

        // Fetches the metadata of a specific asset
        static AssetMetadata& GetMetadata(AssetHandle handle);

        // Retrieves the AssetType from the given extension
        static AssetType GetAssetTypeFromExtension(const String& extension);

        // Checks if an asset Exists in Registry or not
        static bool ExistsInRegistry(const String& absPath);

        // Checks if an asset Exists in Loaded Registry or not
        static bool IsLoaded(const AssetHandle& handle);

        // Returns the pool for loaded assets
        static std::unordered_map<AssetHandle, Ref<Asset>>* GetLoadedAssetsRegistry() { return &sLoadedAssets; }

        // Returns the AssetRegistry
        static AssetRegistry* GetRegistry() { return &sAssetRegistry; }

        // Removes an asset from the registry
        static void RemoveAsset(AssetHandle assetHandle);

        // Returns AssetHandle of a given asset path
        static AssetHandle GetHandleFromPath(const String& assetPath);

        // Creates a brand NEW asset, loads it to RAM and writes it to the registry
        template <typename T, typename... Args>
        static Ref<T> CreateNewAsset(String& assetPath, AssetType type, Args&&... args)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Class must derive from Asset");

            // Convert the path into Relative Path
            std::filesystem::path relativePath = std::filesystem::relative(std::filesystem::path(assetPath), ProjectManager::GetAssetsDirectory());
            String path = relativePath.string();
            std::replace(path.begin(), path.end(), '\\', '/');

            // Create the Metadata for the Asset
            AssetMetadata metadata;
            metadata.Handle = AssetHandle();
            metadata.Path = path; // Relative path is stored
            metadata.Type = type;
            metadata.IsDataLoaded = true;

            // Store the Asset with the metadata in the AssetRegistry
            sAssetRegistry[path] = metadata;

            // Create the Actual Asset that will get used
            Ref<T> asset = T::Create(std::forward<Args>(args)...);

            // Fill in the data for Assets
            asset->SetHandle(metadata.Handle);
            asset->SetType(metadata.Type);
            asset ? asset->SetFlag(AssetFlag::VALID) : asset->SetFlag(AssetFlag::INVALID);

            // Store that in sLoadedAssets(technically in RAM) for quick access
            sLoadedAssets[metadata.Handle] = asset;

            SerializeRegistry();
            return asset;
        }

        // Returns the Asset, from the handle
        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle)
        {
            AssetMetadata& metadata = GetMetadata(assetHandle);

            Ref<Asset> asset = nullptr;
            if (!metadata.IsDataLoaded)
            {
                metadata.IsDataLoaded = AssetLoader::TryLoadData(metadata, asset);
                if (!metadata.IsDataLoaded)
                    return nullptr;

                sLoadedAssets[assetHandle] = asset;
            }
            else
                asset = sLoadedAssets[assetHandle];

            return asset.As<T>();
        }
    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> sLoadedAssets;
        static AssetRegistry sAssetRegistry;
    };
}
