//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "PhysX/extensions/PxDefaultStreams.h"
#include "Core/Buffer.hpp"

namespace Electro
{
    class PhysicsMeshSerializer
    {
    public:
        static bool Exists(const String& meshName, bool isConvex);
        static bool DeleteIfExists(const String& meshName, bool isConvex);
        static void Serialize(const physx::PxDefaultMemoryOutputStream& cookedData, const String& meshName, bool isConvex);
        static Buffer Deserialize(const String& meshName, bool isConvex);
    private:
        static String GetResultantPath(const String& meshName, bool isConvex);
    };
}