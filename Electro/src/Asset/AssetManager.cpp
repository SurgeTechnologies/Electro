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

    // Mapped as { filepath : Resource  }
    static std::unordered_map<String, Ref<Shader>> sShaders;
    static std::unordered_map<String, Ref<Texture2D>> sTextures;
    static std::unordered_map<String, Ref<EnvironmentMap>> sEnvMaps;

    void AssetManager::Init(const String& projectPath)
    {
        sProjectPath = projectPath;
        sAssetManagerInitialized = true;
        Reload();
    }

    void AssetManager::Shutdown()
    {
        sProjectPath.clear();
        ClearAllCache();
        sAssetManagerInitialized = false;
    }

    bool AssetManager::Reload()
    {
        if (!sProjectPath.empty())
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(sProjectPath))
            {
                auto extension = FileSystem::GetExtension(entry.path().string().c_str());
                if (extension == ".png" || extension == ".jpg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr")
                {
                    auto texture = Factory::CreateTexture2D(entry.path().string().c_str());
                    sTextures[entry.path().string()] = texture.Raw();
                }
            }
            return true;
        }
        else
        {
            ELECTRO_WARN("Asset Manager reloading failed, the project path which was selected was empty!");
            return false;
        }
        return false;
    }


    template<typename T> std::unordered_map<String, Ref<T>>& AssetManager::GetMap() { static_assert(false); }
    template<typename T> void AssetManager::Submit(Ref<T>& resource) { static_assert(false) };
    template<typename T> static Ref<T> AssetManager::Get(const String& nameWithExtension) { static_assert(false); }

    /*
     *  GET MAP
     */

    template<> std::unordered_map<String, Ref<Shader>>& AssetManager::GetMap() { return sShaders; }
    template<> std::unordered_map<String, Ref<Texture2D>>& AssetManager::GetMap() { return sTextures; }
    template<> std::unordered_map<String, Ref<EnvironmentMap>>& AssetManager::GetMap() { return sEnvMaps; }

    /*
     *  SUBMIT
     */

    template<> void AssetManager::Submit<Shader>(Ref<Shader>& resource)
    {
        auto filepath = resource->GetFilepath();
        for (auto& cacheShader : sShaders)
            if (cacheShader.first == filepath)
                return;
        sShaders[filepath] = resource;
    }

    template<> void AssetManager::Submit<Texture2D>(Ref<Texture2D>& resource)
    {
        auto filepath = resource->GetFilepath();
        for (auto& cacheTexture : sTextures)
            if (cacheTexture.first == filepath)
                return;
        sTextures[filepath] = resource;
    }

    template<> void AssetManager::Submit<EnvironmentMap>(Ref<EnvironmentMap>& resource)
    {
        auto filepath = resource->GetPath();
        for (auto& cacheEnvMap : sEnvMaps)
            if (cacheEnvMap.first == filepath)
                return;
        sEnvMaps[filepath] = resource;
    }

    /*
     *  GET
     */

    template<> Ref<Shader> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Shader>();
        for (auto& res : resources)
            if (FileSystem::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<Texture2D> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Texture2D>();
        for (auto& res : resources)
            if (FileSystem::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<EnvironmentMap> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<EnvironmentMap>();
        for (auto& res : resources)
            if (FileSystem::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    void AssetManager::ClearAllCache()
    {
        sTextures.clear();
        sShaders.clear();
        sEnvMaps.clear();
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
