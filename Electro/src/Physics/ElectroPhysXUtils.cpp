//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroPhysXUtils.hpp"

namespace Electro::PhysXUtils
{
    physx::PxTransform ToPhysXTransform(const TransformComponent& transform)
    {
        physx::PxQuat r = ToPhysXQuat(glm::normalize(glm::quat(transform.Rotation)));
        physx::PxVec3 p = ToPhysXVector(transform.Translation);
        return physx::PxTransform(p, r);
    }

    physx::PxTransform ToPhysXTransform(const glm::mat4& transform)
    {
        physx::PxQuat r = ToPhysXQuat(glm::normalize(glm::toQuat(transform)));
        physx::PxVec3 p = ToPhysXVector(glm::vec3(transform[3]));
        return physx::PxTransform(p, r);
    }

    physx::PxTransform ToPhysXTransform(const glm::vec3& translation, const glm::vec3& rotation)
    {
        return physx::PxTransform(ToPhysXVector(translation), ToPhysXQuat(glm::quat(rotation)));
    }

    physx::PxMat44 ToPhysXMatrix(const glm::mat4& matrix)
    {
        return *(physx::PxMat44*)&matrix;
    }

    physx::PxVec3 ToPhysXVector(const glm::vec3& vector)
    {
        return *(physx::PxVec3*)&vector;
    }

    physx::PxVec4 ToPhysXVector(const glm::vec4& vector)
    {
        return *(physx::PxVec4*)&vector;
    }

    physx::PxQuat ToPhysXQuat(const glm::quat& quat)
    {
        return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
    }

    glm::mat4 FromPhysXTransform(const physx::PxTransform& transform)
    {
        glm::quat rotation = FromPhysXQuat(transform.q);
        glm::vec3 position = FromPhysXVector(transform.p);
        return glm::translate(glm::mat4(1.0F), position) * glm::toMat4(rotation);
    }

    glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
    {
        return *(glm::mat4*)&matrix;
    }

    glm::vec3 FromPhysXVector(const physx::PxVec3& vector)
    {
        return *(glm::vec3*)&vector;
    }

    glm::vec4 FromPhysXVector(const physx::PxVec4& vector)
    {
        return *(glm::vec4*)&vector;
    }

    glm::quat FromPhysXQuat(const physx::PxQuat& quat)
    {
        return *(glm::quat*)&quat;
    }

    physx::PxFilterFlags ElectroCollisionFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
    {
        if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
        {
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
            return physx::PxFilterFlag::eDEFAULT;
        }

        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
        pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;

        if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
        {
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_CCD;
            return physx::PxFilterFlag::eDEFAULT;
        }

        return physx::PxFilterFlag::eSUPPRESS;
    }

    void PhysicsMeshSerializer::DeleteIfSerialized(const String& filepath)
    {
        std::filesystem::path p = filepath;
        auto dirName = OS::GetNameWithoutExtension(filepath);
        if (IsSerialized(filepath))
            OS::RemoveAll(p.parent_path().string() + "/" + dirName);
    }

    void PhysicsMeshSerializer::SerializeMesh(const String& filepath, const physx::PxDefaultMemoryOutputStream& data, const String& submeshName)
    {
        std::filesystem::path path = OS::GetParentPath(filepath) + "/" + (OS::GetNameWithoutExtension(filepath) + ".pxm");
        auto dirName = OS::GetNameWithoutExtension(filepath);

        if (!submeshName.empty())
            path = OS::GetParentPath(filepath) + "/" + dirName + "/" + (submeshName + ".pxm");

        OS::CreateFolder(OS::GetParentPath(filepath).c_str(), dirName.c_str());
        String cachedFilepath = path.string();
        ELECTRO_TRACE("Serializing %s to %s", submeshName.c_str(), cachedFilepath.c_str());

        FILE* f = fopen(cachedFilepath.c_str(), "wb");
        if (f)
        {
            fwrite(data.getData(), sizeof(physx::PxU8), data.getSize() / sizeof(physx::PxU8), f);
            fclose(f);
        }
        else
            ELECTRO_TRACE("File Already Exists!");
    }

    bool PhysicsMeshSerializer::IsSerialized(const String& filepath)
    {
        return OS::IsDirectory(OS::GetParentPath(filepath) + "/" + OS::GetNameWithoutExtension(filepath));
    }

    static physx::PxU8* sBuffer;
    //Based on; https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/Serialization.html
    physx::PxDefaultMemoryInputData PhysicsMeshSerializer::DeserializeMesh(const String& filepath, const String& submeshName)
    {
        auto dirName = OS::GetNameWithoutExtension(filepath);
        auto path = OS::GetParentPath(filepath) + "/" + dirName;
        if (submeshName.length() > 0)
            path = OS::GetParentPath(filepath) + "/" + dirName + "/" + (submeshName + ".pxm");

        FILE* file = fopen(path.c_str(), "rb");
        Uint size = 0;
        if (file)
        {
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (sBuffer)
                delete[] sBuffer;

            sBuffer = new physx::PxU8[size / sizeof(physx::PxU8)];
            fread(sBuffer, sizeof(physx::PxU8), size / sizeof(physx::PxU8), file);
            fclose(file);
        }

        return physx::PxDefaultMemoryInputData(sBuffer, size);
    }
}
