//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysXInternal.hpp"
#include "ElectroPhysicsActor.hpp"
#include "ElectroPhysXUtils.hpp"

namespace Electro
{
    static PhysicsErrorCallback sErrorCallback;
    static PhysicsAssertHandler sAssertHandler;
    static physx::PxDefaultAllocator sAllocatorCallback;
    static physx::PxFoundation* sFoundation;
    static physx::PxPvd* sPVD;
    static physx::PxPhysics* sPhysics;
    static physx::PxCooking* sCookingFactory;

    void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
        const char* errorMessage = NULL;

        switch (code)
        {
            case physx::PxErrorCode::eNO_ERROR:          errorMessage = "NO ERROR";           break;
            case physx::PxErrorCode::eDEBUG_INFO:        errorMessage = "INFO";               break;
            case physx::PxErrorCode::eDEBUG_WARNING:     errorMessage = "WARNING";            break;
            case physx::PxErrorCode::eINVALID_PARAMETER: errorMessage = "INVALID PARAMETER";  break;
            case physx::PxErrorCode::eINVALID_OPERATION: errorMessage = "INVALID OPERATION";  break;
            case physx::PxErrorCode::eOUT_OF_MEMORY:     errorMessage = "OUT OF MEMORY";      break;
            case physx::PxErrorCode::eINTERNAL_ERROR:    errorMessage = "INTERNAL ERROR";     break;
            case physx::PxErrorCode::eABORT:             errorMessage = "ABORT";              break;
            case physx::PxErrorCode::ePERF_WARNING:      errorMessage = "PERFOMANCE WARNING"; break;
            case physx::PxErrorCode::eMASK_ALL:          errorMessage = "UNKNOWN";            break;
        }

        switch (code)
        {
            case physx::PxErrorCode::eNO_ERROR:
            case physx::PxErrorCode::eDEBUG_INFO:
                ELECTRO_INFO("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eDEBUG_WARNING:
            case physx::PxErrorCode::ePERF_WARNING:
                ELECTRO_WARN("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eINVALID_PARAMETER:
            case physx::PxErrorCode::eINVALID_OPERATION:
            case physx::PxErrorCode::eOUT_OF_MEMORY:
            case physx::PxErrorCode::eINTERNAL_ERROR:
                ELECTRO_ERROR("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eABORT:
            case physx::PxErrorCode::eMASK_ALL:
                ELECTRO_CRITICAL("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); E_INTERNAL_ASSERT("PhysX Terminated..."); break;
        }
    }

    void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
    {
        ELECTRO_CRITICAL("[PhysX Error]: %s:%d - %s", file, line, exp);
    }

    void PhysXInternal::Init()
    {
        E_ASSERT(!sFoundation, "Already initialized internal PhysX!");

        //Setup the foundation
        sFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, sAllocatorCallback, sErrorCallback);
        E_ASSERT(sFoundation, "Cannot create PhysX foundation!");

        //Create a PDV instance
        sPVD = PxCreatePvd(*sFoundation);
        if (sPVD)
        {
            physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
            sPVD->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
        }

        //Create an instance of the PhysX physics SDK
        physx::PxTolerancesScale scale = physx::PxTolerancesScale();
        scale.length = 10;
        sPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *sFoundation, scale, true, sPVD);
        E_ASSERT(sPhysics, "PhysX Physics creation failed!");

        //Create the cooking factory
        sCookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *sFoundation, sPhysics->getTolerancesScale());
        E_ASSERT(sCookingFactory, "PhysX Cooking creation Failed!");

        PxSetAssertHandler(sAssertHandler);
        ELECTRO_INFO("Initialized PhysX");
    }

    void PhysXInternal::ShutDown()
    {
        sCookingFactory->release();
        sPhysics->release();
        sFoundation->release(); //Remember to release() it at the end!
    }

    void PhysXInternal::AddBoxCollider(PhysicsActor& actor)
    {
        auto& collider = actor.mEntity.GetComponent<BoxColliderComponent>();
        glm::vec3 colliderSize = collider.Size;
        glm::vec3 size = actor.mEntity.Transform().Scale;

        if (size.x != 0.0f)
            colliderSize.x *= size.x;
        if (size.y != 0.0f)
            colliderSize.y *= size.y;
        if (size.z != 0.0f)
            colliderSize.z *= size.z;

        physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(colliderSize.x / 2.0f, colliderSize.y / 2.0f, colliderSize.z / 2.0f);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, boxGeometry, *actor.mInternalMaterial);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(PhysXUtils::ToPhysXTransform(glm::translate(glm::mat4(1.0f), collider.Offset)));
    }

    void PhysXInternal::AddSphereCollider(PhysicsActor& actor)
    {
        auto& collider = actor.mEntity.GetComponent<SphereColliderComponent>();
        float colliderRadius = collider.Radius;
        glm::vec3 size = actor.mEntity.Transform().Scale;
        if (size.x != 0.0f)
            colliderRadius *= size.x;

        physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(colliderRadius);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, sphereGeometry, *actor.mInternalMaterial);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
    }

    void PhysXInternal::AddCapsuleCollider(PhysicsActor& actor)
    {
        auto& collider = actor.mEntity.GetComponent<CapsuleColliderComponent>();

        float colliderRadius = collider.Radius;
        float colliderHeight = collider.Height;
        glm::vec3 size = actor.mEntity.Transform().Scale;
        if (size.x != 0.0f)
            colliderRadius *= (size.x / 2.0f);
        if (size.y != 0.0f)
            colliderHeight *= size.y;

        physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(colliderRadius, colliderHeight / 2.0f);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, capsuleGeometry, *actor.mInternalMaterial);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
    }

    void PhysXInternal::AddMeshCollider(PhysicsActor& actor)
    {

    }

    static physx::PxBroadPhaseType::Enum ElectroToPhysXBroadphaseType(BroadphaseType type)
    {
        switch (type)
        {
            case BroadphaseType::SweepAndPrune:     return physx::PxBroadPhaseType::eSAP;
            case BroadphaseType::MultiBoxPrune:     return physx::PxBroadPhaseType::eMBP;
            case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
        }

        return physx::PxBroadPhaseType::eABP;
    }

    static physx::PxFrictionType::Enum ElectroToPhysXFrictionType(FrictionType type)
    {
        switch (type)
        {
            case FrictionType::Patch:           return physx::PxFrictionType::ePATCH;
            case FrictionType::OneDirectional:  return physx::PxFrictionType::eONE_DIRECTIONAL;
            case FrictionType::TwoDirectional:  return physx::PxFrictionType::eTWO_DIRECTIONAL;
        }

        return physx::PxFrictionType::ePATCH;
    }

    physx::PxScene* PhysXInternal::CreateScene()
    {
        physx::PxSceneDesc sceneDesc(sPhysics->getTolerancesScale());

        const PhysicsSettings& settings = PhysicsEngine::GetSettings();

        sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
        sceneDesc.broadPhaseType = ElectroToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
        sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
        sceneDesc.filterShader = PhysXUtils::ElectroFilterShader;
        //sceneDesc.simulationEventCallback = &sContactListener;
        sceneDesc.frictionType = ElectroToPhysXFrictionType(settings.FrictionModel);

        E_ASSERT(sceneDesc.isValid(), "Scene is not valid!");
        return sPhysics->createScene(sceneDesc);
    }

    physx::PxPhysics& PhysXInternal::GetPhysics()
    {
        return *sPhysics;
    }

    physx::PxAllocatorCallback& PhysXInternal::GetAllocator()
    {
        return sAllocatorCallback;
    }
}
