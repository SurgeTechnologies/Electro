//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <foundation/PxErrorCallback.h>
#include <foundation/PxAssert.h>

namespace Electro
{
    class PhysicsErrorCallback : public physx::PxErrorCallback
    {
    public:
        virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
    };

    class PhysicsAssertHandler : public physx::PxAssertHandler
    {
        virtual void operator()(const char* exp, const char* file, int line, bool& ignore);
    };
}
