//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Generator.hpp"
#include "Core/System/OS.hpp"
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

    // Mapped as { name : Resource  }
    static std::unordered_map<String, Ref<Framebuffer>> sFramebuffers;

    void AssetManager::Init(const String& projectPath)
    {
        if (sAssetManagerInitialized)
            AssetManager::Shutdown();

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
                auto extension = OS::GetExtension(entry.path().string().c_str());
                if (extension == ".png" || extension == ".jpg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr")
                {
                    auto texture = EGenerator::CreateTexture2D(entry.path().string().c_str());
                    sTextures[entry.path().string()] = texture.Raw();
                }
                else if (extension == ".ePMat")
                {

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
    template<> std::unordered_map<String, Ref<Framebuffer>>& AssetManager::GetMap() { return sFramebuffers; }
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

    template<> void AssetManager::Submit<Framebuffer>(Ref<Framebuffer>& resource)
    {
        auto name = resource->GetName();
        for (auto& cacheFramebuffer : sFramebuffers)
            if (cacheFramebuffer.first == name)
                return;
        sFramebuffers[name] = resource;
    }

    /*
     *  GET
     */

    template<> Ref<Shader> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Shader>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<Texture2D> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Texture2D>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<EnvironmentMap> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<EnvironmentMap>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<Framebuffer> AssetManager::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Framebuffer>();
        for (auto& res : resources)
            if (res.first.c_str() == nameWithExtension)
                return res.second;
        return nullptr;
    }

    void AssetManager::ClearAllCache()
    {
        sTextures.clear();
        sShaders.clear();
        sFramebuffers.clear();
        sEnvMaps.clear();
    }

    bool AssetManager::IsInitialized()
    {
        return sAssetManagerInitialized;
    }
}
