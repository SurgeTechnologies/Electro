//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
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
                if (GetNameWithExtension(res.first) == nameWithExtension)
                    return res.second;
            return nullptr;
        }

        /* [Spike] Filepath utilities [Spike] */
        static String GetNameWithoutExtension(const String& assetFilepath);
        static String GetNameWithExtension(const String& assetFilepath);
        static String GetExtension(const String& assetFilepath);
        static String GetProjectPath() { return sProjectPath; }

        static bool Exists(const String& nameWithExtension, ResourceType type);
        static bool Exists(const char* path, ResourceType type);
        static bool IsVaultInitialized();

        static Vector<Ref<Shader>> GetAllShaders();
        static Vector<Ref<Texture2D>> GetAllTextures();
        static Vector<String> GetAllDirsInProjectPath();
        static Vector<String> GetAllFilePathsFromParentPath(const String& path);

        /* [Spike] Mapped as { filepath : Resource } [Spike] */
        static std::unordered_map<String, String> GetAllScripts();

        static bool CreateFolder(const char* parentDirectory, const char* name);
        static void ClearAllCache();

        /* [Spike] File Readers [Spike] */
        static String ReadFile(const String& filepath);
        static Vector<char> ReadBinaryFile(const String& filepath);
    private:
        static String sProjectPath; /* [Spike] Base Path, such as: "C:/Users/Dummy/Desktop/SpikeProject" [Spike] */
        static bool sVaultInitialized;

        /* [Spike] Mapped as { filepath : Resource  } [Spike] */
        static std::unordered_map<String, Ref<Shader>> sShaders;
        static std::unordered_map<String, Ref<Texture2D>> sTextures;
        static std::unordered_map<String, String> sScripts;
    };
}
