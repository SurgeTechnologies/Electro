//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/FileSystem.hpp"
#include "PhysicsMeshSerializer.hpp"
#include "Project/ProjectManager.hpp"

#define MESH_EXTENSION ".ePxm"
#define CONVEX_MESH_SUFFIX ".CONVEX" MESH_EXTENSION
#define TRIANGLE_MESH_SUFFIX ".TRIANGLE" MESH_EXTENSION

namespace Electro
{
    bool PhysicsMeshSerializer::Exists(const String& meshName, bool isConvex)
    {
        const String& resultantPath = GetResultantPath(meshName, isConvex);
        return FileSystem::Exists(resultantPath);
    }

    bool PhysicsMeshSerializer::DeleteIfExists(const String& meshName, bool isConvex)
    {
        const String& resultantPath = GetResultantPath(meshName, isConvex);
        bool exists = FileSystem::Exists(resultantPath);
        if (exists)
            FileSystem::Deletefile(resultantPath);
        return exists;
    }

    void PhysicsMeshSerializer::Serialize(const physx::PxDefaultMemoryOutputStream& cookedData, const String& meshName, bool isConvex)
    {
        const String resultantSerializePath = GetResultantPath(meshName, isConvex);

        FILE* f = fopen(resultantSerializePath.c_str(), "wb");
        if (f)
        {
            fwrite(cookedData.getData(), sizeof(physx::PxU8), cookedData.getSize() / sizeof(physx::PxU8), f);
            fclose(f);
        }
        else
            Log::Error("Invalid filepath {0}", resultantSerializePath);
    }

    Buffer PhysicsMeshSerializer::Deserialize(const String& meshName, bool isConvex)
    {
        const String resultantDeserializePath = GetResultantPath(meshName, isConvex);

        FILE* f = fopen(resultantDeserializePath.c_str(), "rb");
        Uint size = 0;

        Buffer binaryCookedData;
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size = ftell(f);
            fseek(f, 0, SEEK_SET);

            binaryCookedData.Allocate(size);
            fread(binaryCookedData.GetData(), sizeof(byte), size / sizeof(byte), f);

            fclose(f);
        }
        else
            Log::Error("Invalid filepath {0}", resultantDeserializePath);

        return binaryCookedData;
    }

    String PhysicsMeshSerializer::GetResultantPath(const String& meshName, bool isConvex)
    {
        const std::filesystem::path& cacheDirectory = ProjectManager::GetPhysicsCacheDirectory();

        String meshNameWithExtension;
        isConvex ? meshNameWithExtension = meshName + CONVEX_MESH_SUFFIX : meshNameWithExtension = meshName + TRIANGLE_MESH_SUFFIX;

        const String& resultantPath = (cacheDirectory / meshNameWithExtension).string();
        return resultantPath;
    }
}