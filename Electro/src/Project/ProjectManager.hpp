//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Log.hpp"
#include "Project.hpp"
#include <filesystem>

namespace Electro
{
    class ProjectManager
    {
    public:
        ProjectManager() = default;
        ~ProjectManager() = default;

        static void SetActive(const Ref<Project>& project);

        static String GetAbsoluteBasePath()
        {
            return sActiveProject->GetConfig().ProjectDirectory;
        }

        static std::filesystem::path GetAssetsDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            const ProjectConfig& config = sActiveProject->GetConfig();

            return (std::filesystem::path(config.ProjectDirectory) / config.AssetsDirectoryName);
        }

        static std::filesystem::path GetProjectDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            return sActiveProject->GetConfig().ProjectDirectory;
        }

        static std::filesystem::path GetScriptAssemblyDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            const ProjectConfig& config = sActiveProject->GetConfig();
            return (std::filesystem::path(config.ProjectDirectory) / config.ScriptDirectoryName);
        }

        static std::filesystem::path GetCacheDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            return (std::filesystem::path(sActiveProject->GetConfig().ProjectDirectory) / "Cache");
        }

        static std::filesystem::path GetPhysicsCacheDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            return (std::filesystem::path(sActiveProject->GetConfig().ProjectDirectory) / "Cache" / "Physics");
        }

        static std::filesystem::path GetAssetRegistryCacheDirectory()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            return (std::filesystem::path(sActiveProject->GetConfig().ProjectDirectory) / "Cache" / "AssetRegistry");
        }

        static std::filesystem::path GetAssetRegistryCachePath()
        {
            E_ASSERT(sActiveProject, "Invalid Active Project!");
            return (std::filesystem::path(sActiveProject->GetConfig().ProjectDirectory) / "Cache" / "AssetRegistry" / "AssetRegistry.exar");
        }

        static Project* GetActiveProjectSlot() { return sActiveProject.Raw(); }
    private:
        inline static Ref<Project> sActiveProject;
    };
}