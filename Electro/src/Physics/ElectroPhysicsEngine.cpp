//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsEngine.hpp"
#include "ElectroPhysXInternal.hpp"

namespace Electro
{
    void PhysicsEngine::Init()
    {
        PhysXInternal::Init();
    }

    void PhysicsEngine::ShutDown()
    {
        PhysXInternal::ShutDown();
    }
}
