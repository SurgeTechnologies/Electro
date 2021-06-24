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

    bool PhysicsMaterial::Serialize()
    {
        std::ofstream out(mPathInDisk, std::ios::binary);
        if (out)
        {
            out.write(reinterpret_cast<const char*>(&mStaticFriction), sizeof(float));
            out.write(reinterpret_cast<const char*>(&mDynamicFriction), sizeof(float));
            out.write(reinterpret_cast<const char*>(&mBounciness), sizeof(float));
            out.close();

            Log::Debug("Serializing PhysicsMaterial...");
            Log::Debug("mStaticFriction is {0}", mStaticFriction);
            Log::Debug("mDynamicFriction is {0}", mDynamicFriction);
            Log::Debug("mBounciness is {0}", mBounciness);
            Log::Debug("Done!");
            return true;
        }
        return false;
    }

    bool PhysicsMaterial::Deserialize()
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

            Log::Debug("Deserializing PhysicsMaterial...");
            Log::Debug("mStaticFriction is {0}", mStaticFriction);
            Log::Debug("mDynamicFriction is {0}", mDynamicFriction);
            Log::Debug("mBounciness is {0}", mBounciness);
            Log::Debug("Done!");
            return true;
        }
        return false;
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
