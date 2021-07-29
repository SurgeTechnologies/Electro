//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ProjectManager.hpp"
#include "Asset/AssetManager.hpp"

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

            FileSystem::CreateOrEnsureFolderExists(GetCacheDirectory().string());
            FileSystem::CreateOrEnsureFolderExists(GetPhysicsCacheDirectory().string());
            FileSystem::CreateOrEnsureFolderExists(GetAssetRegistryCacheDirectory().string());

            AssetManager::Init();
        }
    }
}