//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"

namespace Electro
{
    class PhysicsMaterial : public Asset
    {
    public:
        PhysicsMaterial() = default;
        PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness)
            : StaticFriction(staticFriction), DynamicFriction(dynamicFriction), Bounciness(bounciness) {}

        ~PhysicsMaterial() = default;

        inline float& GetStaticFriction() { return StaticFriction; }
        inline float& GetDynamicFriction() { return DynamicFriction; }
        inline float& GetBounciness() { return Bounciness; }

        inline const float& GetStaticFriction() const { return StaticFriction; }
        inline const float& GetDynamicFriction() const { return DynamicFriction; }
        inline const float& GetBounciness() const { return Bounciness; }

        inline void SetStaticFriction(float staticFriction) { StaticFriction = staticFriction; }
        inline void SetDynamicFriction(float dynamicFriction) { DynamicFriction = dynamicFriction; }
        inline void SetBounciness(float bounciness) { Bounciness = bounciness; }

        inline static Ref<PhysicsMaterial> Create() { return Ref<PhysicsMaterial>::Create(); }
    private:
        float StaticFriction = 0.3f;
        float DynamicFriction = 0.3f;
        float Bounciness = 0.3f;
    };
}
