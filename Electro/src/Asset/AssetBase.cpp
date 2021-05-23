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

    void PhysicsMaterial::Set(const glm::vec3& data)
    {
        mStaticFriction = data.x;
        mDynamicFriction = data.y;
        mBounciness = data.z;
    }

    void PhysicsMaterial::Serialize()
    {
        std::ofstream out(mPathInDisk, std::ios::binary);
        if (out)
        {
            out.write(reinterpret_cast<const char*>(&mStaticFriction), sizeof(float));
            out.write(reinterpret_cast<const char*>(&mDynamicFriction), sizeof(float));
            out.write(reinterpret_cast<const char*>(&mBounciness), sizeof(float));
            out.close();
        }

        ELECTRO_DEBUG("Serializing PhysicsMaterial...");
        ELECTRO_DEBUG("mStaticFriction is %f", mStaticFriction);
        ELECTRO_DEBUG("mDynamicFriction is %f", mDynamicFriction);
        ELECTRO_DEBUG("mBounciness is %f", mBounciness);
        ELECTRO_DEBUG("Done!");
    }

    void PhysicsMaterial::Deserialize()
    {
        std::ifstream in(mPathInDisk);
        if (in)
        {
            in.read(reinterpret_cast<char*>(&mStaticFriction), sizeof(float));
            in.seekg(sizeof(float));
            in.read(reinterpret_cast<char*>(&mDynamicFriction), sizeof(float));
            in.seekg(sizeof(float) * 2);
            in.read(reinterpret_cast<char*>(&mStaticFriction), sizeof(float));
            in.close();
        }
        ELECTRO_INFO("Deserializing PhysicsMaterial...");
        ELECTRO_INFO("mStaticFriction is %f", mStaticFriction);
        ELECTRO_INFO("mDynamicFriction is %f", mDynamicFriction);
        ELECTRO_INFO("mBounciness is %f", mBounciness);
        ELECTRO_INFO("Done!");
    }
}
