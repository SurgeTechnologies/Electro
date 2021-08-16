//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "RuntimeExporter.hpp"
#include "Asset/AssetManager.hpp"

namespace Electro
{
    // TODO: Multithread
    void RuntimeExporter::ExportCurrent(const ExporterOptions& options)
    {
        Project* project = ProjectManager::GetActiveProjectSlot();

        FileSystem::RemoveAll(options.ExportPath);

        // Create necessary files and folders
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath);
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro/Assets");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro/Vendor");
        FileSystem::CreateOrEnsureFolderExists(options.ExportPath + "/Electro/Scripts");

        // Copy the game executable + necessary dlls
        FileSystem::Copy("ElectroRuntime/Bin", options.ExportPath);

        // Copy the engine assets + dependencies
        FileSystem::Copy("Electro/assets/Renderer", options.ExportPath + "/Electro/Assets/Renderer");
        FileSystem::Copy("Electro/assets/Shaders", options.ExportPath + "/Electro/Assets/Shaders");
        FileSystem::Copy("Electro/assets/SPIRvCache", options.ExportPath + "/Electro/Assets/SPIRvCache");

        FileSystem::Copy("Electro/vendor/ElectroMono", options.ExportPath + "/Electro/vendor/ElectroMono");

        // Copy ScriptEngine Binaries
        FileSystem::Copy("ExampleApp/bin/Release", options.ExportPath + "/Electro/Scripts");

        // Copy Game Assets, TODO: strip unnecessary assets, compress
        FileSystem::Copy(ProjectManager::GetAssetsDirectory().string(), options.ExportPath + "/Electro/Assets");
        FileSystem::Copy(ProjectManager::GetCacheDirectory().string(), options.ExportPath + "/Electro/Cache");
        FileSystem::Copy(ProjectManager::GetEPROJFilePath(), options.ExportPath);

        // Rename the files
        FileSystem::RenameFile(options.ExportPath + "/ElectroRuntime.exe", options.ApplicationName);
    }
}
