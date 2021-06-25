//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "AssetBase.hpp"
#include "AssetManager.hpp"
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

    void PhysicsMaterial::Set(const glm::vec3& data)
    {
        mStaticFriction = data.x;
        mDynamicFriction = data.y;
        mBounciness = data.z;
    }

    Ref<PhysicsMaterial> PhysicsMaterial::Create(const String& path)
    {
        Ref<PhysicsMaterial> result = AssetManager::Get<PhysicsMaterial>(AssetManager::GetHandle(path));
        if (!result)
        {
            result = Ref<PhysicsMaterial>::Create(path);
            AssetManager::Submit<PhysicsMaterial>(result);
        }
        return result;
    }
}
