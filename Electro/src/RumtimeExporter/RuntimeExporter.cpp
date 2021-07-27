//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "RuntimeExporter.hpp"
#include "Core/FileSystem.hpp"
#include "Asset/AssetManager.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    void RuntimeExporter::ExportCurrent(const ExporterOptions& options)
    {
        Project* project = ProjectManager::GetActiveProjectSlot();

        FileSystem::RemoveAll(options.ExportPath);

        // Create necessary files and folders
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath);
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro/vendor");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/ExampleApp");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/ExampleApp/bin");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Assets");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Cache");

        // Copy the game executable + necessary dlls
        FileSystem::Copy("ElectroRuntime/Bin", options.ExportPath);

        // Copy the engine assets + dependencies
        FileSystem::Copy("Electro/assets", options.ExportPath + "/Electro/assets");
        FileSystem::Copy("Electro/vendor/ElectroMono", options.ExportPath + "/Electro/vendor/ElectroMono");

        // Copy ScriptEngine Binaries
        FileSystem::Copy("ExampleApp/bin", options.ExportPath + "/ExampleApp/bin");

        // Copy Game Assets, TODO: strip unnecessary assets, compress
        FileSystem::Copy(ProjectManager::GetAssetsDirectory().string(), options.ExportPath + "/Assets");
        FileSystem::Copy(ProjectManager::GetCacheDirectory().string(), options.ExportPath + "/Cache");
        FileSystem::Copy(ProjectManager::GetEPROJFilePath(), options.ExportPath);

        // Rename the files
        FileSystem::RenameFile(options.ExportPath + "/ElectroRuntime.exe", options.ApplicationName);
    }
}
