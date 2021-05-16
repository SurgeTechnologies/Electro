//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"
#include "Core/FileSystem.hpp"
#include <unordered_map>

namespace Electro
{
    class Shader;
    class Texture2D;
    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        static void Init(const String& projectPath);
        static void Shutdown();

        template<typename T>
        static void Submit(const Ref<T>& resource)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");

            //Make sure same asset is not pushed more than once
            for (const auto& [handle, asset] : sRegistry)
                if (handle == resource->mHandle)
                    return;

            E_ASSERT(resource->mHandle.IsValid(), "Invalid asset Handle!");
            sRegistry[resource->mHandle] = resource;
        }

        template<typename T>
        static Ref<T> Get(const AssetHandle& assetID)
        {
            //Validation
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            E_ASSERT(assetID.IsValid(), "Invalid asset Handle!");
            Ref<T> null = nullptr;
            //Trying to find the resource in the registry by comparing the handles
            for (const auto& [handle, asset] : sRegistry)
                if (handle == assetID)
                    return asset.As<T>();

            //Resource is not there, return nullptr
            return null;
        }

        template<typename T>
        static Ref<T> Get(const String& nameWithExtension)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            Ref<T> null = nullptr;
            //Trying to find the resource in the registry by comparing the name
            for (const auto& [handle, asset] : sRegistry)
                if (FileSystem::GetNameWithExtension(asset->mName) == nameWithExtension)
                    return asset.As<T>();

            //Resource is not there, return nullptr
            return null;
        }

        template<typename T>
        static Vector<Ref<T>> GetAll(AssetType type)
        {
            static_assert(std::is_base_of<Asset, T>::value, "Given Type must derive from Asset!");
            Vector<Ref<T>> result;
            for (const auto& [handle, asset] : sRegistry)
                if (asset->mBaseType == type)
                    result.push_back(asset.As<T>());

            return result;
        }

        static AssetHandle GetHandle(const String& nameWithExtension);
        static void Remove(const AssetHandle& assetHandle);

        static String GetProjectPath() { return sProjectPath; }
        static bool IsInitialized();

    private:
        static String sProjectPath;
        static bool sAssetManagerInitialized;
        static std::unordered_map<AssetHandle, Ref<Asset>> sRegistry;
    };
}
