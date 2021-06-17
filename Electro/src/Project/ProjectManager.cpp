//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ProjectManager.hpp"
#include "Core/FileSystem.hpp"

namespace Electro
{
    void ProjectManager::SetActive(const Ref<Project>& project)
    {
        sActiveProject = project;
        if (!project->GetConfig().ProjectDirectory.empty())
        {
            FileSystem::CreateOrEnsureFolderExists(GetAssetsDirectory().string());
            FileSystem::CreateOrEnsureFolderExists((GetAssetsDirectory() / "Scenes").string());
            FileSystem::CreateOrEnsureFolderExists(GetScriptAssemblyDirectory().string());
        }
    }
}