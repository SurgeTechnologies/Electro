//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroVault.hpp"
#include "System/ElectroOS.hpp"

namespace Electro
{
    String Vault::sProjectPath = "";
    bool Vault::sVaultInitialized = false;
    std::unordered_map<String, Ref<Shader>>      Vault::sShaders;
    std::unordered_map<String, Ref<Texture2D>>   Vault::sTextures;
    std::unordered_map<String, Ref<Framebuffer>> Vault::sFramebuffers;

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
        return true; //TODO
    }

    //TODO: Add more resource type by extending these specialization function
    template<typename T> std::unordered_map<String, Ref<T>>& Vault::GetMap() { static_assert(false); }
    template<typename T> void Vault::Submit(Ref<T>& resource) { static_assert(false) };
    template<typename T> static bool Vault::Exists(const String& nameWithExtension) { static_assert(false); }
    template<typename T> static Ref<T> Vault::Get(const String& nameWithExtension) { static_assert(false); }

    template<> std::unordered_map<String, Ref<Shader>>& Vault::GetMap() { return sShaders; }
    template<> std::unordered_map<String, Ref<Texture2D>>& Vault::GetMap() { return sTextures; }
    template<> std::unordered_map<String, Ref<Framebuffer>>& Vault::GetMap() { return sFramebuffers; }

    template<> bool Vault::Exists<Shader>(const String& nameWithExtension)
    {
        for (auto& shader : sShaders)
            if (OS::GetNameWithExtension(shader.first.c_str()) == nameWithExtension)
                return true;
        return false;
    }

    template<> bool Vault::Exists<Texture2D>(const String& nameWithExtension)
    {
        for (auto& texture : sTextures)
            if (OS::GetNameWithExtension(texture.first.c_str()) == nameWithExtension)
                return true;
        return false;
    }

    template<> bool Vault::Exists<Framebuffer>(const String& nameWithExtension)
    {
        for (auto& fb : sFramebuffers)
            if (fb.first.c_str() == nameWithExtension)
                return true;
        return false;
    }

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
    }

    bool Vault::IsVaultInitialized()
    {
        return sVaultInitialized;
    }
}