//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ProjectSerializer.hpp"
#include "Core/FileSystem.hpp"
#include "ProjectManager.hpp"
#include <yaml-cpp/yaml.h>

#define SCENE_KEY "Scene"
namespace Electro
{
    void ProjectSerializer::Serialize(const Project* project) const
    {
        const ProjectConfig& config = project->GetConfig();
        E_ASSERT(!config.ProjectDirectory.empty(), "Empty Project directory!");

        YAML::Emitter out;

        out << YAML::BeginMap; // Electro Project
        out << YAML::Key << "ProjectName"          << YAML::Value << config.ProjectName;
        out << YAML::Key << "ProjectDirectory"     << YAML::Value << config.ProjectDirectory;
        out << YAML::Key << "AssetsDirectoryName"  << YAML::Value << config.AssetsDirectoryName;
        out << YAML::Key << "ScriptsDirectoryName" << YAML::Value << config.ScriptDirectoryName;

        const Uint totalScenes = static_cast<Uint>(config.ScenePaths.size());

        out << YAML::Key << "Total Scenes" << totalScenes;
        for (Uint i = 0; i < totalScenes; i++)
            out << YAML::Key << (SCENE_KEY + std::to_string(i)) << YAML::Value << config.ScenePaths[i];

        FileSystem::WriteFile(ProjectManager::GetEPROJFilePath(), out.c_str());
    }

    bool ProjectSerializer::Deserialize(Project* outProject, const String& filePath) const
    {
        ProjectConfig& config = outProject->GetConfig();

        YAML::Node data;
        try { data = YAML::LoadFile(filePath); }
        catch (const YAML::ParserException& ex)
        {
            Log::Error("Failed to load .electro file '{0}'\n  {0}", filePath, ex.what());
            return false;
        }

        config.ProjectName = data["ProjectName"].as<String>();
        config.ProjectDirectory = data["ProjectDirectory"].as<String>();
        config.AssetsDirectoryName = data["AssetsDirectoryName"].as<String>();
        config.ScriptDirectoryName = data["ScriptsDirectoryName"].as<String>();
        const Uint totalSceneSize = data["Total Scenes"].as<Uint>();

        config.ScenePaths.resize(totalSceneSize);
        for(Uint i = 0; i < totalSceneSize; i++)
        {
            config.ScenePaths[i] = data[SCENE_KEY + std::to_string(i)].as<String>();
        }

        return true;
    }
}
