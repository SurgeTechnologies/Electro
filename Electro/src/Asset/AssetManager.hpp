//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/FileSystem.hpp"
#include "Asset/AssetBase.hpp"
#include <unordered_map>

namespace Electro
{
    //Forward decls of the assets that are cached
    class Texture2D;
    class EnvironmentMap;
    class PhysicsMaterial;

    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        static void Init();
        static void Shutdown();

        template<typename T>
        static void Submit(const Ref<T>& resource)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            const AssetHandle& resourceHandle = resource->GetHandle();
            E_ASSERT(resourceHandle.IsValid(), "Invalid asset Handle!");

            // Make sure same asset is not pushed more than once
            for (const auto& [handle, asset] : sLoadedAssets)
                if (handle == resourceHandle)
                    return;

            sLoadedAssets[resourceHandle] = resource.As<Asset>();
        }

        template<typename T>
        static Ref<T> Get(const AssetHandle& assetID)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");

            // Trying to find the resource in the registry by comparing the handles
            for (const auto& [handle, asset] : sLoadedAssets)
            {
                if (handle == assetID)
                    return asset.As<T>();
            }

            // Resource is not there, return nullptr
            return Ref<T>(nullptr);
        }

        template<typename T>
        static Vector<Ref<T>> GetAll(AssetType type)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            Vector<Ref<T>> result;

            for (const auto& [handle, asset] : sLoadedAssets)
                if (asset->GetType() == type)
                    result.push_back(asset.As<T>());

            return result;
        }

        static bool Exists(const String& path);
        static bool Exists(const AssetHandle& handle);
        static AssetHandle GetHandle(const String& path);

        static bool Remove(const String& path);
        static bool Remove(const AssetHandle& assetHandle);

        //static void SerializeAssetRegistry(); // TODO
    private:
        static std::unordered_map<AssetHandle, Ref<Asset>> sLoadedAssets;
    };
}
