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

    physx::PxFilterFlags ElectroFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
    {
        if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
        {
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
            return physx::PxFilterFlag::eDEFAULT;
        }

        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

        if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
        {
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
            pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT | physx::PxPairFlag::eNOTIFY_TOUCH_CCD;
            return physx::PxFilterFlag::eDEFAULT;
        }

        return physx::PxFilterFlag::eSUPPRESS;
    }

    void PhysicsMeshSerializer::DeleteIfSerialized(const String& filepath)
    {
        std::filesystem::path p = filepath;
        auto dirName = OS::GetNameWithoutExtension(filepath);
        if (IsSerialized(filepath))
            std::filesystem::remove_all(p.parent_path() / dirName);
    }

    //TODO: Rework PhysicsMeshSerializer
    void PhysicsMeshSerializer::SerializeMesh(const String& filepath, const physx::PxDefaultMemoryOutputStream& data, const String& submeshName)
    {
        std::filesystem::path p = filepath;
        std::filesystem::path path = p.parent_path() / (p.filename().string() + ".pxm");
        auto dirName = OS::GetNameWithoutExtension(filepath);

        if (submeshName.length() > 0)
            path = p.parent_path() / dirName / (submeshName + ".pxm");

        std::filesystem::create_directory(p.parent_path() / dirName);
        String cachedFilepath = path.string();

        ELECTRO_INFO("Serializing %s to %s", submeshName.c_str(), cachedFilepath.c_str());

        FILE* f = fopen(cachedFilepath.c_str(), "wb");
        if (f)
        {
            ELECTRO_INFO("File Created");
            fwrite(data.getData(), sizeof(physx::PxU8), data.getSize() / sizeof(physx::PxU8), f);
            fclose(f);
        }
        else
            ELECTRO_INFO("File Already Exists");
    }

    bool PhysicsMeshSerializer::IsSerialized(const String& filepath)
    {
        std::filesystem::path p = filepath;
        auto dirName = OS::GetNameWithoutExtension(filepath);
        auto path = p.parent_path() / dirName;
        return std::filesystem::is_directory(path);
    }

    static physx::PxU8* sMeshDataBuffer;

    physx::PxDefaultMemoryInputData PhysicsMeshSerializer::DeserializeMesh(const String& filepath, const String& submeshName)
    {
        std::filesystem::path p = filepath;
        auto dirName = OS::GetNameWithoutExtension(filepath);

        auto path = p.parent_path() / dirName;
        if (submeshName.length() > 0)
            path = p.parent_path() / dirName / (submeshName + ".pxm");

        FILE* f = fopen(path.string().c_str(), "rb");
        Uint size = 0;

        if (f)
        {
            fseek(f, 0, SEEK_END);
            size = ftell(f);
            fseek(f, 0, SEEK_SET);

            if (sMeshDataBuffer)
                delete[] sMeshDataBuffer;

            sMeshDataBuffer = new physx::PxU8[size / sizeof(physx::PxU8)];
            fread(sMeshDataBuffer, sizeof(physx::PxU8), size / sizeof(physx::PxU8), f);
            fclose(f);
        }

        return physx::PxDefaultMemoryInputData(sMeshDataBuffer, size);
    }
}
