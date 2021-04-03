//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsEngine.hpp"
#include "ElectroPhysicsActor.hpp"
#include "ElectroPhysXInternal.hpp"
#include "ElectroPhysXUtils.hpp"

namespace Electro
{
    static PhysicsSettings sPhysicsSettings;
    static physx::PxScene* sScene;
    static Vector<Ref<PhysicsActor>> sActors;
    static float sSimulationTime = 0.0f;

    void PhysicsEngine::Init()
    {
        PhysXInternal::Init();
    }

    void PhysicsEngine::ShutDown()
    {
        PhysXInternal::ShutDown();
    }

    void PhysicsEngine::CreateScene()
    {
        E_ASSERT(sScene == nullptr, "Scene already has a Physics Scene!");
        sScene = PhysXInternal::CreateScene();

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

    Ref<PhysicsActor> PhysicsEngine::CreateActor(Entity e)
    {
        E_ASSERT(sScene, "Scene is not valid!");
        Ref<PhysicsActor> actor = Ref<PhysicsActor>::Create(e);
        sActors.push_back(actor);
        actor->Spawn();
        return actor;
    }

    Ref<PhysicsActor> PhysicsEngine::GetActorForEntity(Entity entity)
    {
        for (auto& actor : sActors)
            if (actor->GetEntity() == entity)
                return actor;
        return nullptr;
    }

    void PhysicsEngine::Simulate(Timestep ts)
    {
        sSimulationTime += ts.GetMilliseconds();

        if (sSimulationTime < sPhysicsSettings.FixedTimestep)
            return;

        sSimulationTime -= sPhysicsSettings.FixedTimestep;

        sScene->simulate(sPhysicsSettings.FixedTimestep);
        sScene->fetchResults(true);

        for (auto& actor : sActors)
            actor->UpdateTransform();
    }

    void PhysicsEngine::DestroyScene()
    {
        E_ASSERT(sScene, "Scene is not valid!");
        sActors.clear();
        sScene->release();
        sScene = nullptr;
    }

    void* PhysicsEngine::GetPhysicsScene()
    {
        return sScene;
    }

    PhysicsSettings& PhysicsEngine::GetSettings()
    {
        return sPhysicsSettings;
    }
}
