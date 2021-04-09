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

        template<typename T> static std::unordered_map<String, Ref<T>>& GetMap();
        template<typename T> static void Submit(Ref<T>& resource);
        template<typename T> static bool Exists(const String& nameWithExtension);
        template<typename T> static Ref<T> Get(const String& nameWithExtension);

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
