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

    enum class FilterGroup : Uint
    {
        Static = BIT(0),
        Dynamic = BIT(1),
        Kinematic = BIT(2),
        All = Static | Dynamic | Kinematic
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

    struct PhysicsSettings
    {
        float FixedTimestep = 0.02f;
        glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f };
        BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
        glm::vec3 WorldBoundsMin = glm::vec3(0.0f);
        glm::vec3 WorldBoundsMax = glm::vec3(1.0f);
        Uint WorldBoundsSubdivisions = 2;
        FrictionType FrictionModel = FrictionType::Patch;
        Uint SolverIterations = 6;
        Uint SolverVelocityIterations = 1;
    };

    class PhysicsEngine
    {
    public:
        static void Init();
        static void ShutDown();

        static void CreateScene();
        static Ref<PhysicsActor> CreateActor(Entity e);
        static Ref<PhysicsActor> GetActorForEntity(Entity entity);
        static void Simulate(Timestep ts);
        static void DestroyScene();
        static void* GetPhysicsScene();
        static PhysicsSettings& GetSettings();
    };
}