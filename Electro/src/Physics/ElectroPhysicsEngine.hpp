//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/ElectroEntity.hpp"

namespace Electro
{
    class PhysicsActor;
    enum class ForceMode : uint16_t
    {
        Force = 0,
        Impulse,
        VelocityChange,
        Acceleration
    };

    enum class BroadphaseType
    {
        SweepAndPrune,
        MultiBoxPrune,
        AutomaticBoxPrune
    };

    enum class FrictionType
    {
        Patch,
        OneDirectional,
        TwoDirectional
    };

    struct RaycastHit
    {
        float Distance;
        uint64_t EntityUUID;
        glm::vec3 Position;
        glm::vec3 Normal;
    };

    struct PhysicsSettings
    {
        float FixedTimestep = 0.02f;
        glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f };
        BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
        FrictionType FrictionModel = FrictionType::Patch;
        glm::vec3 WorldBoundsMin = glm::vec3(0.0f);
        glm::vec3 WorldBoundsMax = glm::vec3(1.0f);
        Uint WorldBoundsSubdivisions = 2;
        Uint SolverIterations = 6;
        Uint SolverVelocityIterations = 1;
        PhysicsMaterialComponent GlobalPhysicsMaterial;
    };

    class PhysicsEngine
    {
    public:
        static void Init();
        static void ShutDown();

        static Ref<PhysicsActor> CreateActor(Entity e);
        static Ref<PhysicsActor> GetActorForEntity(const Entity& entity);
        static bool Raycast(RaycastHit* hit, const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
        static void Simulate(Timestep ts);
        static void* GetPhysicsScene();
        static PhysicsSettings& GetSettings();
        static PhysicsMaterialComponent& GetGlobalPhysicsMaterial();
    };

    class PhysicsSceneSlot
    {
    public:
        static void CreateScene();
        static void DestroyScene();
    };
}