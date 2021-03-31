//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Scene/ElectroEntity.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"

namespace physx
{
    class PxRigidActor;
    class PxShape;
    class PxMaterial;
}

namespace Electro
{
    class PhysicsActor : public IElectroRef
    {
    public:
        PhysicsActor(Entity entity);
        ~PhysicsActor();

        Entity& GetEntity() { return mEntity; }
        float GetMass() const;
        void SetMass(float mass);

        glm::vec3 GetPosition();
        glm::quat GetRotation();

        void AddForce(const glm::vec3& force, ForceMode forceMode);
        void AddTorque(const glm::vec3& torque, ForceMode forceMode);

        glm::vec3 GetLinearVelocity() const;
        void SetLinearVelocity(const glm::vec3& velocity);

        glm::vec3 GetAngularVelocity() const;
        void SetAngularVelocity(const glm::vec3& velocity);

        void SetLinearDrag(float drag) const;
        void SetAngularDrag(float drag) const;
        void SetLayer(uint32_t layerId);
        bool IsDynamic() const { return mRigidBody.BodyType == RigidBodyComponent::Type::Dynamic; }
    private:
        void Initialize();
        void Spawn();
        void SynchronizeTransform();
        void AddCollisionShape(physx::PxShape* shape);
    private:
        Entity mEntity;
        RigidBodyComponent& mRigidBody;
        PhysicsMaterialComponent mPhysicsMaterial;

        physx::PxRigidActor* mInternalActor;
        std::unordered_map<int, std::vector<physx::PxShape*>> mShapes;
        physx::PxMaterial* mInternalMaterial;

        friend class PhysicsEngine;
        friend class PhysXInternal;
    };
}