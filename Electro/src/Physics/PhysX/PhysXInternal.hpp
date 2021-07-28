//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Components.hpp"
#include <PhysX/PxPhysicsAPI.h>
#include "Physics/ContactListener.hpp"

namespace Electro
{
    struct RaycastHit;
    class PhysicsActor;

    class PhysXInternal
    {
    public:
        static void Init();
        static void Shutdown();

        static void AddBoxCollider(PhysicsActor& actor);
        static void AddSphereCollider(PhysicsActor& actor);
        static void AddCapsuleCollider(PhysicsActor& actor);
        static void AddMeshCollider(PhysicsActor& actor);

        static physx::PxPhysics& GetPhysics();
        static physx::PxAllocatorCallback& GetAllocator();
        static physx::PxDefaultCpuDispatcher& GetCpuDispatcher();
        static ContactListener& GetContactListener();

        static Vector<physx::PxShape*> CreateConvexMesh(MeshColliderComponent& collider, const glm::vec3& size, bool generateDebugLayout = false);
        static Vector<physx::PxShape*> CreateTriangleMesh(MeshColliderComponent& collider, const glm::vec3& scale, bool generateDebugLayout = false);
        static void GenerateDebugMesh(MeshColliderComponent& collider, const Vector<physx::PxShape*>& shapes, bool isConvex);
    };
}
