//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"

namespace Electro
{
    class PhysicsMaterial : public IElectroRef
    {
    public:
        PhysicsMaterial() = default;
        PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness)
            : StaticFriction(staticFriction), DynamicFriction(dynamicFriction), Bounciness(bounciness) {}

        ~PhysicsMaterial() = default;

        inline float& GetStaticFriction() { return StaticFriction; }
        inline float& GetDynamicFriction() { return DynamicFriction; }
        inline float& GetBounciness() { return Bounciness; }

        inline void SetStaticFriction(float staticFriction) { StaticFriction = staticFriction; }
        inline void SetDynamicFriction(float dynamicFriction) { DynamicFriction = dynamicFriction; }
        inline void SetBounciness(float bounciness) { Bounciness = bounciness; }

    private:
        float StaticFriction = 0.3f;
        float DynamicFriction = 0.3f;
        float Bounciness = 0.3f;
    };
}
