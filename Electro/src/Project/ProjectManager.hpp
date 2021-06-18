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
    private:
        inline static Ref<Project> sActiveProject;
    };
}