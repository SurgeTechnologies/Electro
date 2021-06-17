//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Ref.hpp"

namespace Electro
{
    struct ProjectConfig
    {
        String ProjectName;
        Vector<String> ScenePaths;
        String ScriptDirectoryName = "Scripts";
        String AssetsDirectoryName = "Assets";

        String ProjectDirectory; // Root of the Project
    };

    class Project : public IElectroRef
    {
    public:
        Project() = default;
        Project(const ProjectConfig& config) { mConfig = config; };
        ~Project() = default;

        const ProjectConfig& GetConfig() const { return mConfig; }
    private:
        ProjectConfig mConfig;
    };
}