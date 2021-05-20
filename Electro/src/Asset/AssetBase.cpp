//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "AssetBase.hpp"
#include "Core/FileSystem.hpp"

namespace Electro
{
    void Asset::SetupAssetBase(const String& filepath, AssetType type, const String& name)
    {
        mHandle.MakeValid();
        mPathInDisk = filepath;
        mName = name.empty() ? FileSystem::GetNameWithExtension(filepath) : name;
        mExtension = FileSystem::GetExtension(filepath);
        mBaseType = type;
    }

    PhysicsMaterial::PhysicsMaterial(const String& path)
    {
        SetupAssetBase(path, AssetType::PhysicsMaterial);
    }

    PhysicsMaterial::PhysicsMaterial()
    {
        SetupAssetBase("[Built In]", AssetType::PhysicsMaterial, "Global Physics Material");
    }
}
