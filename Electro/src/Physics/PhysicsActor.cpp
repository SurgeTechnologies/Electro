//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysicsActor.hpp"
#include "PhysXInternal.hpp"
#include "PhysicsActor.hpp"
#include "Scripting/ScriptEngine.hpp"
#include <glm/gtx/compatibility.hpp>

namespace Electro
{
    PhysicsActor::PhysicsActor(Entity entity)
        : mEntity(entity), mRigidBody(mEntity.GetComponent<RigidBodyComponent>())
    {
        // Entity must have a RigidBodyComponent
        Ref<PhysicsMaterial> pmat = mRigidBody.PhysicsMaterial;
        if (pmat)
            mPhysicsMaterial = pmat;
        else
            mPhysicsMaterial = PhysicsEngine::GetGlobalPhysicsMaterial();
        Initialize();
    }

    PhysicsActor::~PhysicsActor()
    {
        EPX_RELEASE(mInternalMaterial);
        EPX_RELEASE(mInternalActor);
    }

    void PhysicsActor::Rotate(const glm::vec3& rotation)
    {
        physx::PxTransform transform = mInternalActor->getGlobalPose();
        transform.q *= (physx::PxQuat(glm::radians(rotation.x), { 1.0f, 0.0f, 0.0f }) * physx::PxQuat(glm::radians(rotation.y), { 0.0f, 1.0f, 0.0f }) * physx::PxQuat(glm::radians(rotation.z), { 0.0f, 0.0f, 1.0f }));
        mInternalActor->setGlobalPose(transform);
    }

    float PhysicsActor::GetMass() const
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to access mass of a NonDynamic-Rigidbody!");
            return 0.0f;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return actor->getMass();
    }

    void PhysicsActor::SetMass(float mass)
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to set mass of a NonDynamic-Rigidbody!");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
        mRigidBody.Mass = mass;
    }

    void PhysicsActor::AddForce(const glm::vec3& force, ForceMode forceMode)
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to add force to a NonDynamic-Rigidbody!");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->addForce(PhysXUtils::ToPhysXVector(force), (physx::PxForceMode::Enum)forceMode);
    }

    void PhysicsActor::AddTorque(const glm::vec3& torque, ForceMode forceMode)
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to add torque to a NonDynamic-Rigidbody!");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->addTorque(PhysXUtils::ToPhysXVector(torque), (physx::PxForceMode::Enum)forceMode);
    }

    glm::vec3 PhysicsActor::GetAngularVelocity() const
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to get angular velocity of a NonDynamic-Rigidbody!");
            return glm::vec3(0.0f);
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return PhysXUtils::FromPhysXVector(actor->getAngularVelocity());
    }

    void PhysicsActor::SetAngularVelocity(const glm::vec3& velocity)
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to set angular velocity to a NonDynamic-Rigidbody!");
            return;
        }

        if (!glm::all(glm::isfinite(velocity)))
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setAngularVelocity(PhysXUtils::ToPhysXVector(velocity));
    }

    void PhysicsActor::SetAngularDrag(float drag) const
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to set angular drag to a NonDynamic-Rigidbody!");
            return;
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setAngularDamping(drag);
    }

    void PhysicsActor::Update(float fixedTimestep)
    {
        if (!ScriptEngine::IsEntityModuleValid(mEntity))
            return;
        ScriptEngine::OnFixedUpdate(mEntity, fixedTimestep);
    }

    glm::vec3 PhysicsActor::GetLinearVelocity() const
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to get linear velocity of a NonDynamic-Rigidbody!");
            return glm::vec3(0.0f);
        }

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        return PhysXUtils::FromPhysXVector(actor->getLinearVelocity());
    }

    void PhysicsActor::SetLinearVelocity(const glm::vec3& velocity)
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to set linear velocity to a NonDynamic-Rigidbody!");
            return;
        }

        if (!glm::all(glm::isfinite(velocity)))
            return;

        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setLinearVelocity(PhysXUtils::ToPhysXVector(velocity));
    }

    void PhysicsActor::SetLinearDrag(float drag) const
    {
        if (!IsDynamic())
        {
            Log::Warn("Trying to set linear drag to a NonDynamic-Rigidbody!");
            return;
        }
        physx::PxRigidDynamic* actor = (physx::PxRigidDynamic*)mInternalActor;
        actor->setLinearDamping(drag);
    }

    void PhysicsActor::Initialize()
    {
        physx::PxPhysics& physics = PhysXInternal::GetPhysics();

        if (mRigidBody.BodyType == RigidBodyComponent::Type::Static)
            mInternalActor = physics.createRigidStatic(PhysXUtils::ToPhysXTransform(mEntity.Transform()));
        else
        {
            const PhysicsSettings& settings = PhysicsEngine::GetSettings();

            physx::PxRigidDynamic* actor = physics.createRigidDynamic(PhysXUtils::ToPhysXTransform(mEntity.Transform()));
            actor->setLinearDamping(mRigidBody.LinearDrag);
            actor->setAngularDamping(mRigidBody.AngularDrag);
            actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, mRigidBody.IsKinematic);
            actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, (bool)mRigidBody.CollisionDetectionMode);
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

        mInternalMaterial = physics.createMaterial(mPhysicsMaterial->mStaticFriction, mPhysicsMaterial->mDynamicFriction, mPhysicsMaterial->mBounciness);

        if (mEntity.HasComponent<BoxColliderComponent>())
            PhysXInternal::AddBoxCollider(*this);
        if (mEntity.HasComponent<SphereColliderComponent>())
            PhysXInternal::AddSphereCollider(*this);
        if (mEntity.HasComponent<CapsuleColliderComponent>())
            PhysXInternal::AddCapsuleCollider(*this);
        if (mEntity.HasComponent<MeshColliderComponent>())
            PhysXInternal::AddMeshCollider(*this);

        //Set the simulation filter data
        physx::PxAllocatorCallback& allocator = PhysXInternal::GetAllocator();
        physx::PxFilterData filterData;
        filterData.word0 = E_BIT(0);
        filterData.word1 = E_BIT(0);
        const physx::PxU32 numShapes = mInternalActor->getNbShapes();
        physx::PxShape** shapes = (physx::PxShape**)allocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
        mInternalActor->getShapes(shapes, numShapes);
        for (physx::PxU32 i = 0; i < numShapes; i++)
            shapes[i]->setSimulationFilterData(filterData);
        allocator.deallocate(shapes);
        mInternalActor->userData = &mEntity;
    }

    void PhysicsActor::SubmitToPhysX()
    {
        ((physx::PxScene*)PhysicsEngine::GetPhysicsScene())->addActor(*mInternalActor);
    }

    void PhysicsActor::UpdateTransform()
    {
        if (IsDynamic())
        {
            TransformComponent& transform = mEntity.Transform();
            physx::PxTransform actorPose = mInternalActor->getGlobalPose();
            transform.Translation = PhysXUtils::FromPhysXVector(actorPose.p);
            transform.Rotation = glm::eulerAngles(PhysXUtils::FromPhysXQuat(actorPose.q));
        }
        else
            mInternalActor->setGlobalPose(PhysXUtils::ToPhysXTransform(mEntity.Transform()));
    }
}
