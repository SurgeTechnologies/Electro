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
    class Shader;
    class Texture2D;
    class EnvironmentMap;
    class PhysicsMaterial;
    class Material;

    template<typename T>
    using AssetRegistry = std::unordered_map<AssetHandle, Ref<T>>;

    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        static void Init(const String& projectPath);
        static void Shutdown();

        template<typename T>
        static void Submit(const Ref<T> resource)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            AssetRegistry<T>& registry = GetRegistry<T>();

            //Make sure same asset is not pushed more than once
            for (const auto& [handle, asset] : registry)
                if (handle == resource->mHandle)
                    return;

            E_ASSERT(resource->mHandle.IsValid(), "Invalid asset Handle!");
            registry[resource->mHandle] = resource;
        }

        template<typename T>
        static Ref<T> Get(const AssetHandle& assetID)
        {
            //Validation
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");

            Ref<T> null = nullptr;
            AssetRegistry<T>& registry = GetRegistry<T>();

            //Trying to find the resource in the registry by comparing the handles
            for (const auto& [handle, asset] : registry)
                if (handle == assetID)
                    return asset;

            //Resource is not there, return nullptr
            return null;
        }

        template<typename T>
        static Ref<T> Get(const String& nameWithExtension)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");

            Ref<T> null = nullptr;
            AssetRegistry<T>& registry = GetRegistry<T>();

            //Trying to find the resource in the registry by comparing the name
            for (const auto& [handle, asset] : registry)
                if (FileSystem::GetNameWithExtension(asset->mPathInDisk) == nameWithExtension)
                    return asset;

            //Resource is not there, return nullptr
            return null;
        }

        template<typename T>
        static Vector<Ref<T>> GetAll(AssetType type)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            Vector<Ref<T>> result;
            AssetRegistry<T>& registry = GetRegistry<T>();

            for (const auto& [handle, asset] : registry)
                if (asset->mBaseType == type)
                    result.push_back(asset);

            return result;
        }

        template<typename T>
        static bool Exists(const String& path)
        {
            AssetHandle handle = GetHandle<T>(path);
            if (handle.IsValid())
            {
                AssetRegistry<T>& registry = GetRegistry<T>();
                if (registry.find(handle) == registry.end())
                    return false; //Asset is not in registry
                else
                    return true;
            }
            return false;
        }

        template<typename T>
        static AssetHandle GetHandle(const String& path)
        {
            AssetRegistry<T>& registry = GetRegistry<T>();
            for (const auto& [handle, asset] : registry)
                if (asset->mPathInDisk == path)
                    return handle;

            AssetHandle nullHandle;
            nullHandle.MakeInvalid();
            return nullHandle;
        }

        template<typename T>
        static bool Remove(const String& path)
        {
            const AssetHandle handle = GetHandle<T>(path);
            return Remove<T>(handle);
        }

        template<typename T>
        static bool Remove(const AssetHandle& assetHandle)
        {
            AssetRegistry<T>& registry = GetRegistry<T>();
            if (assetHandle.IsValid())
            {
                registry.erase(assetHandle);
                return true;
            }
            return false;
        }

        static void RemoveIfExists(const String& path);

        static String GetProjectPath() { return sProjectPath; }
        static bool IsInitialized();
    private:
        template<typename T>
        static AssetRegistry<T>& GetRegistry()
        {
            if constexpr (std::is_same_v<T, Shader>)
                return sShaderRegistry;
            else if constexpr (std::is_same_v<T, Texture2D>)
                return sTexture2DRegistry;
            else if constexpr (std::is_same_v<T, EnvironmentMap>)
                return sEnvMapRegistry;
            else if constexpr (std::is_same_v<T, Material>)
                return sMaterialRegistry;
            else if constexpr (std::is_same_v<T, PhysicsMaterial>)
                return sPhysicsMaterialRegistry;
            else
                static_assert(false, "No Registry found for given type!");
        }

    private:
        static String sProjectPath;
        static bool sAssetManagerInitialized;

        static AssetRegistry<Shader> sShaderRegistry;
        static AssetRegistry<Texture2D> sTexture2DRegistry;
        static AssetRegistry<EnvironmentMap> sEnvMapRegistry;
        static AssetRegistry<Material> sMaterialRegistry;
        static AssetRegistry<PhysicsMaterial> sPhysicsMaterialRegistry;
    };
}
