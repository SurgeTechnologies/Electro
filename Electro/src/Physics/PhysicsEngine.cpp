//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysicsEngine.hpp"
#include "PhysicsActor.hpp"
#include "PhysX/PhysXInternal.hpp"
#include "PhysX/PhysXUtils.hpp"

namespace Electro
{
    static PhysicsSettings sPhysicsSettings;
    static physx::PxScene* sScene;
    static Vector<Ref<PhysicsActor>> sActors;
    static float sSimulationTime = 0.0f;

    void PhysicsEngine::Init()
    {
        PhysXInternal::Init();
        sPhysicsSettings.GlobalPhysicsMaterial = Ref<PhysicsMaterial>::Create();
        sPhysicsSettings.GlobalPhysicsMaterial->SetStaticFriction(0.3f);
        sPhysicsSettings.GlobalPhysicsMaterial->SetDynamicFriction(0.3f);
        sPhysicsSettings.GlobalPhysicsMaterial->SetBounciness(0.4f);
    }

    void PhysicsEngine::ShutDown()
    {
        PhysXInternal::Shutdown();
    }

    Ref<PhysicsActor> PhysicsEngine::CreateActor(Entity e)
    {
        E_ASSERT(sScene, "Scene is not valid!");
        Ref<PhysicsActor> actor = Ref<PhysicsActor>::Create(e);
        sActors.push_back(actor);
        actor->AddToScene();
        return actor;
    }

    Ref<PhysicsActor> PhysicsEngine::GetActorForEntity(const Entity& entity)
    {
        for (auto& actor : sActors)
        {
            if (actor->GetEntity() == entity)
                return actor;
        }

        return nullptr;
    }

    bool PhysicsEngine::Raycast(RaycastHit* hit, const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
    {
        physx::PxRaycastBuffer hitResult;
        bool status = sScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(direction), maxDistance, hitResult);
        if (status)
        {
            Entity& entity = *(Entity*)hitResult.block.actor->userData;
            auto tag = entity.GetComponent<TagComponent>().Tag;
            hit->EntityUUID = entity.GetUUID();
            hit->Distance = hitResult.block.distance;
            hit->Position = PhysXUtils::FromPhysXVector(hitResult.block.position);
            hit->Normal = PhysXUtils::FromPhysXVector(hitResult.block.normal);
        }
        return status;
    }

    void PhysicsEngine::Simulate(Timestep ts)
    {
        sSimulationTime += ts.GetMilliseconds();

        if (sSimulationTime < sPhysicsSettings.FixedTimestep)
            return;

        sSimulationTime -= sPhysicsSettings.FixedTimestep;

        for (Ref<PhysicsActor>& actor : sActors)
            actor->Update(sPhysicsSettings.FixedTimestep);

        sScene->simulate(sPhysicsSettings.FixedTimestep);
        sScene->fetchResults(true);

        for (Ref<PhysicsActor>& actor : sActors)
            actor->UpdateTransform();
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

    void PhysicsSceneSlot::CreateScene()
    {
        E_ASSERT(sScene == nullptr, "Scene already has a Physics Scene!");

        physx::PxPhysics& pxPhysics = PhysXInternal::GetPhysics();

        physx::PxSceneDesc sceneDesc(pxPhysics.getTolerancesScale());
        sceneDesc.gravity                 = PhysXUtils::ToPhysXVector(sPhysicsSettings.Gravity);
        sceneDesc.filterShader            = PhysXUtils::ElectroCollisionFilterShader;
        sceneDesc.cpuDispatcher           = &PhysXInternal::GetCpuDispatcher();
        sceneDesc.simulationEventCallback = &PhysXInternal::GetContactListener();
        sceneDesc.broadPhaseType          = ElectroToPhysXBroadphaseType(sPhysicsSettings.BroadphaseAlgorithm);
        sceneDesc.frictionType            = ElectroToPhysXFrictionType(sPhysicsSettings.FrictionModel);
        sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD; // Enable continuous collision detection

        E_ASSERT(sceneDesc.isValid(), "Scene is not valid!");
        sScene = pxPhysics.createScene(sceneDesc);

        if (sPhysicsSettings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            physx::PxBounds3* regionBounds = nullptr;
            physx::PxBounds3 globalBounds(PhysXUtils::ToPhysXVector(sPhysicsSettings.WorldBoundsMin), PhysXUtils::ToPhysXVector(sPhysicsSettings.WorldBoundsMax));
            Uint regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, sPhysicsSettings.WorldBoundsSubdivisions);

            for (Uint i = 0; i < regionCount; i++)
            {
                physx::PxBroadPhaseRegion region;
                region.bounds = regionBounds[i];
                sScene->addBroadPhaseRegion(region);
            }
        }
    }

    void PhysicsSceneSlot::DestroyScene()
    {
        E_ASSERT(sScene, "Scene is not valid!");
        for (auto& actor : sActors)
            actor.Reset();

        sActors.clear();
        EPX_RELEASE(sScene);
    }

    void* PhysicsEngine::GetPhysicsScene()
    {
        return sScene;
    }

    PhysicsSettings& PhysicsEngine::GetSettings()
    {
        return sPhysicsSettings;
    }

    Ref<PhysicsMaterial>& PhysicsEngine::GetGlobalPhysicsMaterial()
    {
        return sPhysicsSettings.GlobalPhysicsMaterial;
    }
}
