//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroVault.hpp"
#include "EDevice/EDevice.hpp"
#include "System/ElectroOS.hpp"
#include "Renderer/ElectroEnvironmentMap.hpp"
#include "Renderer/Interface/ElectroShader.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Interface/ElectroFramebuffer.hpp"

namespace Electro
{
    String Vault::sProjectPath = "";
    bool Vault::sVaultInitialized = false;
    // Mapped as { filepath : Resource  }
    static std::unordered_map<String, Ref<Shader>> sShaders;
    static std::unordered_map<String, Ref<Texture2D>> sTextures;
    static std::unordered_map<String, Ref<EnvironmentMap>> sEnvMaps;
    // Mapped as { name : Resource  }
    static std::unordered_map<String, Ref<Framebuffer>> sFramebuffers;

    void Vault::Init(const String& projectPath)
    {
        if (sVaultInitialized)
            Vault::Shutdown();

        sProjectPath = projectPath;
        sVaultInitialized = true;
        Reload();
    }

    void Vault::Shutdown()
    {
        sProjectPath.clear();
        ClearAllCache();
    }

    bool Vault::Reload()
    {
        if (!sProjectPath.empty())
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(sProjectPath))
            {
                auto extension = OS::GetExtension(entry.path().string().c_str());
                if (extension == ".png" || extension == ".jpg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr")
                {
                    auto texture = EDevice::CreateTexture2D(entry.path().string().c_str());
                    sTextures[entry.path().string()] = texture.Raw();
                }
            }
            return true;
        }
        else
        {
            ELECTRO_WARN("Vault reloading failed, the project path which was selected was empty!");
            return false;
        }
        return false;
    }

    //TODO: Add more resource type by extending these specialization function
    template<typename T> std::unordered_map<String, Ref<T>>& Vault::GetMap() { static_assert(false); }
    template<typename T> void Vault::Submit(Ref<T>& resource) { static_assert(false) };
    template<typename T> static Ref<T> Vault::Get(const String& nameWithExtension) { static_assert(false); }

    template<> std::unordered_map<String, Ref<Shader>>& Vault::GetMap() { return sShaders; }
    template<> std::unordered_map<String, Ref<Texture2D>>& Vault::GetMap() { return sTextures; }
    template<> std::unordered_map<String, Ref<Framebuffer>>& Vault::GetMap() { return sFramebuffers; }
    template<> std::unordered_map<String, Ref<EnvironmentMap>>& Vault::GetMap() { return sEnvMaps; }

    template<> void Vault::Submit<Shader>(Ref<Shader>& resource)
    {
        auto filepath = resource->GetFilepath();
        for (auto& cacheShader : sShaders)
            if (cacheShader.first == filepath)
                return;
        sShaders[filepath] = resource;
    }

    template<> void Vault::Submit<Texture2D>(Ref<Texture2D>& resource)
    {
        auto filepath = resource->GetFilepath();
        for (auto& cacheTexture : sTextures)
            if (cacheTexture.first == filepath)
                return;
        sTextures[filepath] = resource;
    }

    template<> void Vault::Submit<EnvironmentMap>(Ref<EnvironmentMap>& resource)
    {
        auto filepath = resource->GetPath();
        for (auto& cacheEnvMap : sEnvMaps)
            if (cacheEnvMap.first == filepath)
                return;
        sEnvMaps[filepath] = resource;
    }

    template<> void Vault::Submit<Framebuffer>(Ref<Framebuffer>& resource)
    {
        auto name = resource->GetName();
        for (auto& cacheFramebuffer : sFramebuffers)
            if (cacheFramebuffer.first == name)
                return;
        sFramebuffers[name] = resource;
    }

    template<> Ref<Shader> Vault::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Shader>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<Texture2D> Vault::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Texture2D>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<EnvironmentMap> Vault::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<EnvironmentMap>();
        for (auto& res : resources)
            if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                return res.second;
        return nullptr;
    }

    template<> Ref<Framebuffer> Vault::Get(const String& nameWithExtension)
    {
        const auto& resources = GetMap<Framebuffer>();
        for (auto& res : resources)
            if (res.first.c_str() == nameWithExtension)
                return res.second;
        return nullptr;
    }

    Vector<Ref<Shader>> Vault::GetAllShaders()
    {
        Vector<Ref<Shader>> shaders;
        shaders.resize(sShaders.size());
        for (auto& shader : sShaders)
            shaders.emplace_back(shader.second);

        return shaders;
    }

    Vector<Ref<Texture2D>> Vault::GetAllTextures()
    {
        Vector<Ref<Texture2D>> textures;
        textures.resize(sTextures.size());
        for (auto& texture : sTextures)
            textures.emplace_back(texture.second);

        return textures;
    }

    void Vault::ClearAllCache()
    {
        sTextures.clear();
        sShaders.clear();
        sFramebuffers.clear();
        sEnvMaps.clear();
    }

    bool Vault::IsVaultInitialized()
    {
        return sVaultInitialized;
    }
}
