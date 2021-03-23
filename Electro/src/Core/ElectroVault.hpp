//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroShader.hpp"
#include "Renderer/ElectroTexture.hpp"
#include <unordered_map>

namespace Electro
{
    enum class ResourceType
    {
        SHADER = 0, TEXTURE, SCRIPT
    };

    class Vault
    {
    public:
        Vault() = default;
        ~Vault() = default;

        static void Init(const String& projectPath);
        static void Shutdown();
        static bool Reload();

        template<typename T>
        static std::unordered_map<String, Ref<T>>& GetMap()
        {
            if constexpr (std::is_same_v<T, Shader>)
                return sShaders;
            if constexpr (std::is_same_v<T, Texture2D>)
                return sTextures;
            E_INTERNAL_ASSERT("Unknown Resource type");
        }

        template <typename T>
        static void Submit(Ref<T>& resource)
        {
            if constexpr (std::is_same_v<T, Shader>)
            {
                auto filepath = resource->GetFilepath();
                for (auto& cacheShader : sShaders)
                    if (cacheShader.first == filepath)
                        return;
                sShaders[filepath] = resource;
            }
            if constexpr (std::is_same_v<T, Texture2D>)
            {
                auto filepath = resource->GetFilepath();
                for (auto& cacheTexture : sTextures)
                    if (cacheTexture.first == filepath)
                        return;
                sTextures[filepath] = resource;
            }
        }

        template <typename T>
        static Ref<T> Get(const String& nameWithExtension)
        {
            const std::unordered_map<String, Ref<T>>& resources = GetMap<T>();
            for (auto& res : resources)
                if (OS::GetNameWithExtension(res.first.c_str()) == nameWithExtension)
                    return res.second;
            return nullptr;
        }

        template <typename T>
        static bool Exists(const String& nameWithExtension)
        {
            if constexpr (std::is_same_v<T, Shader>)
            {
                for (auto& shader : sShaders)
                    if (OS::GetNameWithExtension(shader.first.c_str()) == nameWithExtension)
                        return true;
            }
            if constexpr (std::is_same_v<T, Texture2D>)
            {
                for (auto& texture : sTextures)
                    if (OS::GetNameWithExtension(texture.first.c_str()) == nameWithExtension)
                        return true;
            }
            return false;
        }

        static String GetProjectPath() { return sProjectPath; }
        static bool IsVaultInitialized();
        static Vector<Ref<Shader>> GetAllShaders();
        static Vector<Ref<Texture2D>> GetAllTextures();
        static void ClearAllCache();
    private:
        static String sProjectPath; // Base Path, such as: "C:/Users/Dummy/Desktop/ElectroProject"
        static bool sVaultInitialized;

        // Mapped as { filepath : Resource  }
        static std::unordered_map<String, Ref<Shader>> sShaders;
        static std::unordered_map<String, Ref<Texture2D>> sTextures;
    };
}
