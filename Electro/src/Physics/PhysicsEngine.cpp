//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysicsEngine.hpp"
#include "PhysicsActor.hpp"
#include "PhysX/PhysXInternal.hpp"

namespace Electro
{
    struct PhysicsEngineData
    {
        float Accumulator = 0.0f;
        Uint NumSubSteps = 0;
    };

    static PhysicsSettings sPhysicsSettings;
    static Vector<Ref<PhysicsActor>> sActors;
    static PhysicsEngineData sPhysicsEngineData;
    static physx::PxScene* sScene;

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
        physx::PxRaycastBuffer hitBuffer;
        bool status = sScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(direction), maxDistance, hitBuffer);
        if (status)
        {
            Entity& entity = *(Entity*)hitBuffer.block.actor->userData;
            auto tag = entity.GetComponent<TagComponent>().Tag;
            hit->EntityUUID = entity.GetUUID();
            hit->Distance = hitBuffer.block.distance;
            hit->Position = PhysXUtils::FromPhysXVector(hitBuffer.block.position);
            hit->Normal = PhysXUtils::FromPhysXVector(hitBuffer.block.normal);
        }
        return status;
    }

    bool PhysicsEngine::Advance(float dt)
    {
        SubstepStrategy(dt);

        if (sPhysicsEngineData.NumSubSteps == 0)
            return false;

        for (Uint i = 0; i < sPhysicsEngineData.NumSubSteps; i++)
        {
            sScene->simulate(sPhysicsSettings.FixedTimestep);
            sScene->fetchResults(true);
        }

        return true;
    }

    void PhysicsEngine::SubstepStrategy(float ts)
    {
        if (sPhysicsEngineData.Accumulator > sPhysicsSettings.FixedTimestep)
            sPhysicsEngineData.Accumulator = 0.0f;

        sPhysicsEngineData.Accumulator += ts;
        if (sPhysicsEngineData.Accumulator < sPhysicsSettings.FixedTimestep)
        {
            sPhysicsEngineData.NumSubSteps = 0;
            return;
        }

        sPhysicsEngineData.NumSubSteps = glm::min(static_cast<Uint>(sPhysicsEngineData.Accumulator / sPhysicsSettings.FixedTimestep), sPhysicsSettings.MaxSubSteps);
        sPhysicsEngineData.Accumulator -= static_cast<float>(sPhysicsEngineData.NumSubSteps) * sPhysicsSettings.FixedTimestep;
    }

    void PhysicsEngine::Simulate(Timestep ts)
    {
        bool advanced = Advance(ts);
        if (advanced)
        {
            for (auto& actor : sActors)
                actor->UpdateTransform();
        }
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
