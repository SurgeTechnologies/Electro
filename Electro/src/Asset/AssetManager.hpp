//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/System/OS.hpp"
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
        static bool Reload();

        template<typename T> static void Submit(Ref<T>& resource);
        template<typename T> static Ref<T> Get(const String& nameWithExtension);

        static String GetProjectPath() { return sProjectPath; }
        static bool IsInitialized();
        static void ClearAllCache();
    private:
        template<typename T> static std::unordered_map<String, Ref<T>>& GetMap();

    private:
        static String sProjectPath;
        static bool sAssetManagerInitialized;
    };
}
