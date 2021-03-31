//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsActor.hpp"
#include "ElectroPhysXInternal.hpp"
#include "ElectroPhysXUtils.hpp"
#include "ElectroPhysicsActor.hpp"
#include "ElectroPhysicsLayer.hpp"
#include <PhysX/PxPhysicsAPI.h>
#include <glm/gtx/compatibility.hpp>

namespace Electro
{
    PhysicsActor::PhysicsActor(Entity entity)
        :mEntity(entity), mRigidBody(mEntity.GetComponent<RigidBodyComponent>())
    {
        if (!mEntity.HasComponent<PhysicsMaterialComponent>())
        {
            mPhysicsMaterial.StaticFriction = 1.0f;
            mPhysicsMaterial.DynamicFriction = 1.0f;
            mPhysicsMaterial.Bounciness = 0.0f;
        }
        else
            mPhysicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();

        Initialize();
    }

    PhysicsActor::~PhysicsActor()
    {
        if (mInternalActor && mInternalActor->isReleasable())
        {
            mInternalActor->release();
            mInternalActor = nullptr;
        }
    }

    float PhysicsActor::GetMass() const
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to set mass of non-dynamic PhysicsActor... returning 0.0f");
            return 0.0f;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return actor->getMass();
    }

    void PhysicsActor::SetMass(float mass)
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to set mass of non-dynamic PhysicsActor... SetMass() aborted");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
        mRigidBody.Mass = mass;
    }

    glm::vec3 PhysicsActor::GetPosition()
    {
        return PhysicsUtils::FromPhysXVector(mInternalActor->getGlobalPose().p);
    }

    glm::quat PhysicsActor::GetRotation()
    {
        return PhysicsUtils::FromPhysXQuat(mInternalActor->getGlobalPose().q);
    }

    void PhysicsActor::AddForce(const glm::vec3& force, ForceMode forceMode)
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to add force to non-dynamic PhysicsActor.");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->addForce(PhysicsUtils::ToPhysXVector(force), (physx::PxForceMode::Enum)forceMode);
    }

    void PhysicsActor::AddTorque(const glm::vec3& torque, ForceMode forceMode)
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to add torque to non-dynamic PhysicsActor.");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->addTorque(PhysicsUtils::ToPhysXVector(torque), (physx::PxForceMode::Enum)forceMode);
    }

    glm::vec3 PhysicsActor::GetAngularVelocity() const
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to get angular velocity of non-dynamic PhysicsActor.");
            return glm::vec3(0.0f);
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return PhysicsUtils::FromPhysXVector(actor->getAngularVelocity());
    }

    void PhysicsActor::SetAngularVelocity(const glm::vec3& velocity)
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to set angular velocity of non-dynamic PhysicsActor.");
            return;
        }

        if (!glm::all(glm::isfinite(velocity)))
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setAngularVelocity(PhysicsUtils::ToPhysXVector(velocity));
    }

    void PhysicsActor::SetAngularDrag(float drag) const
    {
        if (!IsDynamic())
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setAngularDamping(drag);
    }

    void PhysicsActor::SetLayer(uint32_t layerId)
    {
        physx::PxAllocatorCallback& allocator = PhysXInternal::GetAllocator();
        const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layerId);

        if (layerInfo.CollidesWith == 0)
            return;

        physx::PxFilterData filterData;
        filterData.word0 = layerInfo.BitValue;
        filterData.word1 = layerInfo.CollidesWith;

        const physx::PxU32 numShapes = mInternalActor->getNbShapes();
        physx::PxShape** shapes = (physx::PxShape**)allocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
        mInternalActor->getShapes(shapes, numShapes);

        for (physx::PxU32 i = 0; i < numShapes; i++)
            shapes[i]->setSimulationFilterData(filterData);

        allocator.deallocate(shapes);
    }

    glm::vec3 PhysicsActor::GetLinearVelocity() const
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to get velocity of non-dynamic PhysicsActor.");
            return glm::vec3(0.0f);
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return PhysicsUtils::FromPhysXVector(actor->getLinearVelocity());
    }

    void PhysicsActor::SetLinearVelocity(const glm::vec3& velocity)
    {
        if (!IsDynamic())
        {
            ELECTRO_WARN("Trying to set velocity of non-dynamic PhysicsActor.");
            return;
        }

        if (!glm::all(glm::isfinite(velocity)))
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setLinearVelocity(PhysicsUtils::ToPhysXVector(velocity));
    }

    void PhysicsActor::SetLinearDrag(float drag) const
    {
        if (!IsDynamic())
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setLinearDamping(drag);
    }

    void PhysicsActor::Initialize()
    {
        physx::PxPhysics& physics = PhysXInternal::GetPhysics();

        if (mRigidBody.BodyType == RigidBodyComponent::Type::Static)
        {
            mInternalActor = physics.createRigidStatic(PhysicsUtils::ToPhysXTransform(mEntity.Transform()));
        }
        else
        {
            const PhysicsSettings& settings = PhysicsEngine::GetSettings();

            physx::PxRigidDynamic* actor = physics.createRigidDynamic(PhysicsUtils::ToPhysXTransform(mEntity.Transform()));
            actor->setLinearDamping(mRigidBody.LinearDrag);
            actor->setAngularDamping(mRigidBody.AngularDrag);
            actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, mRigidBody.IsKinematic);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X,  mRigidBody.LockPositionX);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y,  mRigidBody.LockPositionY);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z,  mRigidBody.LockPositionZ);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, mRigidBody.LockRotationX);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, mRigidBody.LockRotationY);
            actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, mRigidBody.LockRotationZ);
            actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, mRigidBody.DisableGravity);
            actor->setSolverIterationCounts(settings.SolverIterations, settings.SolverVelocityIterations);

            physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mRigidBody.Mass);
            mInternalActor = actor;
        }

        mInternalMaterial = physics.createMaterial(mPhysicsMaterial.StaticFriction, mPhysicsMaterial.DynamicFriction, mPhysicsMaterial.Bounciness);
        if (mEntity.HasComponent<BoxColliderComponent>())
            PhysXInternal::AddBoxCollider(*this);

        SetLayer(mRigidBody.Layer);
        mInternalActor->userData = &mEntity;
    }

    void PhysicsActor::Spawn()
    {
        ((physx::PxScene*)PhysicsEngine::GetPhysicsScene())->addActor(*mInternalActor);
    }

    void PhysicsActor::AddCollisionShape(physx::PxShape* shape)
    {
        bool status = mInternalActor->attachShape(*shape);
        shape->release();
        if (!status)
            shape = nullptr;
    }

    void PhysicsActor::SynchronizeTransform()
    {
        if (IsDynamic())
        {
            TransformComponent& transform = mEntity.Transform();
            physx::PxTransform actorPose = mInternalActor->getGlobalPose();
            transform.Translation = PhysicsUtils::FromPhysXVector(actorPose.p);
            transform.Rotation = glm::eulerAngles(PhysicsUtils::FromPhysXQuat(actorPose.q));
        }
        else
        {
            mInternalActor->setGlobalPose(PhysicsUtils::ToPhysXTransform(mEntity.Transform()));
        }
    }
}