//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroVault.hpp"
#include <filesystem>

namespace Electro
{
    String Vault::sProjectPath = "";
    bool Vault::sVaultInitialized = false;
    std::unordered_map<String, Ref<Shader>>             Vault::sShaders;
    std::unordered_map<String, Ref<Texture2D>>          Vault::sTextures;
    std::unordered_map<String, String>                  Vault::sScripts;

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
            if (GetExtension(entry.path().string()) == ".glsl")
            {
                auto shader = Shader::Create(entry.path().string().c_str());
                sShaders[entry.path().string()] = shader.Raw();
            }
            if (GetExtension(entry.path().string()) == ".png")
            {
                auto texture = Texture2D::Create(entry.path().string().c_str());
                sTextures[entry.path().string()] = texture.Raw();
            }
            if (GetExtension(entry.path().string()) == ".jpg")
            {
                auto texture = Texture2D::Create(entry.path().string().c_str());
                sTextures[entry.path().string()] = texture.Raw();
            }
            if (GetExtension(entry.path().string()) == ".cs")
            {
                sScripts[entry.path().string()] = ReadFile(entry.path().string());
            }
        }
        return true;
    }

    String Vault::GetNameWithoutExtension(const String& assetFilepath)
    {
        String name;
        auto lastSlash = assetFilepath.find_last_of("/\\");
        lastSlash = lastSlash == String::npos ? 0 : lastSlash + 1;
        auto lastDot = assetFilepath.rfind('.');
        auto count = lastDot == String::npos ? assetFilepath.size() - lastSlash : lastDot - lastSlash;
        name = assetFilepath.substr(lastSlash, count);
        return name;

    }

    bool Vault::Exists(const String& nameWithExtension, ResourceType type)
    {
        switch (type)
        {
        case ResourceType::SHADER:
            for (auto& shader : sShaders)
                if (GetNameWithExtension(shader.first) == nameWithExtension)
                    return true;

        case ResourceType::TEXTURE:
            for (auto& texture : sTextures)
                if (GetNameWithExtension(texture.first) == nameWithExtension)
                    return true;

        case ResourceType::SCRIPT:
            for (auto& script : sScripts)
                if (GetNameWithExtension(script.first) == nameWithExtension)
                    return true;
        }
        return false;
    }

    bool Vault::Exists(const char* path, ResourceType type)
    {
        switch (type)
        {
        case ResourceType::SHADER:
            for (auto& shader : sShaders)
                if (shader.first == path)
                    return true;

        case ResourceType::TEXTURE:
            for (auto& texture : sTextures)
                if (texture.first == path)
                    return true;

        case ResourceType::SCRIPT:
            for (auto& script : sScripts)
                if (script.first == path)
                    return true;
        }
        return false;
    }

    Vector<Ref<Shader>> Vault::GetAllShaders()
    {
        Vector<Ref<Shader>> shaders;
        shaders.resize(sShaders.size());
        for (auto& shader : sShaders)
        {
            shaders.emplace_back(shader.second);
        }
        return shaders;
    }

    Vector<Ref<Texture>> Vault::GetAllTextures()
    {
        Vector<Ref<Texture>> textures;
        textures.resize(sTextures.size());
        for (auto& texture : sTextures)
        {
            textures.emplace_back(texture.second);
        }
        return textures;
    }

    Vector<String> Vault::GetAllDirsInProjectPath()
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(sProjectPath))
            paths.push_back(entry.path().string());

        return paths;
    }

    Vector<String> Vault::GetAllFilePathsFromParentPath(const String& path)
    {
        Vector<String> names;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            names.push_back(entry.path().string());

        return names;
    }

    std::unordered_map<String, String> Vault::GetAllScripts()
    {
        return sScripts;
    }

    bool Vault::CreateFolder(const char* parentDirectory, const char* name)
    {
        String path = String(parentDirectory) + "/" + String(name);
        if (std::filesystem::create_directory(path) || std::filesystem::exists(path))
            return true;
        return false;
    }

    void Vault::ClearAllCache()
    {
        sTextures.clear();
        sShaders.clear();
        sScripts.clear();
    }

    String Vault::ReadFile(const String& filepath)
    {
        String result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
        }
        else
            ELECTRO_ERROR("Could not open file path \"%s\"", filepath.c_str());

        return result;
    }

    Vector<char> Vault::ReadBinaryFile(const String& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream)
            ELECTRO_ERROR("Cannot open filepath: %s!", filepath.c_str());

        auto end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        auto size = std::size_t(end - stream.tellg());
        if (size == 0) return {};

        Vector<char> buffer(size);
        if (!stream.read((char*)buffer.data(), buffer.size()))
            ELECTRO_ERROR("Cannot read file: %s", filepath.c_str());
        return buffer;
    }

    String Vault::GetExtension(const String& assetFilepath)
    {
        return std::filesystem::path(assetFilepath).extension().string();
    }

    String Vault::GetNameWithExtension(const String& assetFilepath)
    {
        return std::filesystem::path(assetFilepath).filename().string();
    }

    bool Vault::IsVaultInitialized()
    {
        return sVaultInitialized;
    }
}