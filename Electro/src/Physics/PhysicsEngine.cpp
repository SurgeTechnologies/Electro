//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysicsEngine.hpp"
#include "PhysicsActor.hpp"
#include "PhysXInternal.hpp"
#include "PhysXUtils.hpp"

namespace Electro
{
    static PhysicsSettings sPhysicsSettings;
    static physx::PxScene* sScene;
    static Vector<Ref<PhysicsActor>> sActors;
    static float sSimulationTime = 0.0f;

    void PhysicsEngine::Init()
    {
        PhysXInternal::Init();
        sPhysicsSettings.GlobalPhysicsMaterial = Ref<PhysicsMaterial>::Create("Global Physics Material");
        sPhysicsSettings.GlobalPhysicsMaterial->mStaticFriction = 0.1f;
        sPhysicsSettings.GlobalPhysicsMaterial->mDynamicFriction = 0.1f;
        sPhysicsSettings.GlobalPhysicsMaterial->mBounciness = 0.1f;
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
        actor->Submit();
        return actor;
    }

    Ref<PhysicsActor> PhysicsEngine::GetActorForEntity(const Entity& entity)
    {
        for (auto& actor : sActors)
            if (actor->GetEntity() == entity)
                return actor;
        return nullptr;
    }

    bool PhysicsEngine::Raycast(RaycastHit* hit, const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
    {
        return PhysXInternal::Raycast(hit, origin, direction, maxDistance);
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

    void PhysicsSceneSlot::CreateScene()
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
