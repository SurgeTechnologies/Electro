//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroVault.hpp"
#include "System/ElectroOS.hpp"

namespace Electro
{
    String Vault::sProjectPath = "";
    bool Vault::sVaultInitialized = false;
    std::unordered_map<String, Ref<Shader>>             Vault::sShaders;
    std::unordered_map<String, Ref<Texture2D>>          Vault::sTextures;

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
        Vault::ClearAllCache();
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sProjectPath))
        {
            if (OS::GetExtension(entry.path().string().c_str()) == ".glsl")
            {
                auto shader = Shader::Create(entry.path().string().c_str());
                sShaders[entry.path().string()] = shader.Raw();
            }
            if (OS::GetExtension(entry.path().string().c_str()) == ".png")
            {
                auto texture = Texture2D::Create(entry.path().string().c_str());
                sTextures[entry.path().string()] = texture.Raw();
            }
            if (OS::GetExtension(entry.path().string().c_str()) == ".jpg")
            {
                auto texture = Texture2D::Create(entry.path().string().c_str());
                sTextures[entry.path().string()] = texture.Raw();
            }
        }
        return true;
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