//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/FileSystem.hpp"
#include "Asset/AssetBase.hpp"
#include "AssetRegistry.hpp"

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

        // Imports all the asset(s) from the Assets Directory
        static void Load();

        // Writes the registry cache to the file
        static void SerializeRegistry();

        // Loads the registry cache from the file
        static void DeserializeRegistry();

        // Reloads the data for a specific Asset
        static bool ReloadData(AssetHandle assetHandle);

        // Returns the Absolute Path from the relative path stored in the metadata
        static String GetAbsolutePath(const AssetMetadata& metadata);

        // Imports a specific asset
        static AssetHandle ImportAsset(const String& filepath);

        // Fetches the metadata of a specific asset
        static AssetMetadata& GetMetadata(AssetHandle handle);

        // Assr Manager - Only for debug purposes
        static void OnImGuiRender(bool* open);

        // Creates a brand NEW asset, loads it to RAM and writes it to the registry
        template <typename T, typename... Args>
        static Ref<T> CreateNewAsset(const String& assetPath, const AssetType& type, const Args&&... args)
        {
        #if E_DEBUG
            static_assert(std::is_base_of<Asset, T>::value, fmt::format("{0} class must derive from Asset", typeid(T).name()).c_str());
        #endif

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
            Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
            asset->SetHandle(metadata.Handle);

            // Store that in sLoadedAssets(technically in RAM) for quick access
            sLoadedAssets[asset->GetHandle()] = asset;

            SerializeRegistry();
            return asset;
        }

        // Returns the Asset, from the handle
        template<typename T>
        static Ref<T> GetAsset(AssetHandle assetHandle)
        {
            const AssetMetadata& metadata = GetMetadata(assetHandle);

            Ref<Asset> asset = nullptr;
            if (!metadata.IsDataLoaded)
            {
                metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);
                if (!metadata.IsDataLoaded)
                    return nullptr;

                sLoadedAssets[assetHandle] = asset;
            }
            else
                asset = sLoadedAssets[assetHandle];

            return asset.As<T>();
        }
    private:
        // Imports all the assets from the given direcotry 
        static void ProcessDirectory(const String& directoryPath);

        // Retrieves the AssetType from the given extension
        static AssetType GetAssetTypeFromExtension(const String& str);
    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> sLoadedAssets;
        static AssetRegistry sAssetRegistry;
    };
}
