//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Project.hpp"

namespace Electro
{
    class ProjectSerializer
    {
    public:
        void Serialize(const Project* project) const;
        bool Deserialize(Project* outProject, const String& filePath) const;
    };
}
