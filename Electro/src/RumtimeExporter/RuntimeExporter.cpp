//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "RuntimeExporter.hpp"
#include "Core/FileSystem.hpp"
#include "Asset/AssetManager.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    void RuntimeExporter::ExportCurrent(const String& path)
    {
        Project* project = ProjectManager::GetActiveProjectSlot();

        std::filesystem::remove_all(path);

        // Create - Copy all necessary files and folders
        FileSystem::CreateOrEnsureFolderExists(path);
        FileSystem::CreateOrEnsureFolderExists(path + "/Electro");
        FileSystem::CreateOrEnsureFolderExists(path + "/Electro/vendor");
        FileSystem::CreateOrEnsureFolderExists(path + "/ExampleApp");
        FileSystem::CreateOrEnsureFolderExists(path + "/ExampleApp/bin");
        FileSystem::CreateOrEnsureFolderExists(path + "/Assets");

        std::filesystem::copy_options options = std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

        std::filesystem::copy("ElectroRuntime/Bin", path, options);
        std::filesystem::copy("Electro/assets", path + "/Electro/assets", options);
        std::filesystem::copy("Electro/vendor/ElectroMono", path + "/Electro/vendor/ElectroMono", options);
        std::filesystem::copy("ExampleApp/bin", path + "/ExampleApp/bin", options);

        std::filesystem::copy(ProjectManager::GetAssetsDirectory(), path + "/Assets", options);
        std::filesystem::copy(ProjectManager::GetEPROJFilePath(), path, options);
    }
}
