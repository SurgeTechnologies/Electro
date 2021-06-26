//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Components.hpp"
#include <PhysX/PxPhysicsAPI.h>

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

        static bool Raycast(RaycastHit* hit, const glm::vec3& origin, const glm::vec3& direction, float maxDistance);

        static physx::PxScene* CreateScene();
        static physx::PxPhysics& GetPhysics();
        static physx::PxAllocatorCallback& GetAllocator();

        static Vector<physx::PxShape*> CreateConvexMesh(MeshColliderComponent& collider, const glm::vec3& size, bool generateDebugLayout = false);
        static Vector<physx::PxShape*> CreateTriangleMesh(MeshColliderComponent& collider, const glm::vec3& scale, bool generateDebugLayout = false);
        static void GenerateDebugMesh(MeshColliderComponent& collider, const Vector<physx::PxShape*>& shapes, bool isConvex);
    };

    class PhysicsErrorCallback : public physx::PxErrorCallback
    {
    public:
        virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
    };

    class PhysicsAssertHandler : public physx::PxAssertHandler
    {
        virtual void operator()(const char* exp, const char* file, int line, bool& ignore);
    };

    class ContactListener : public physx::PxSimulationEventCallback
    {
    public:
        virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
        virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
        virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
    };
}
