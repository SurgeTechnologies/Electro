//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>
#include "Core/Buffer.hpp"

namespace Electro
{
    class Material;
    class AssetSerializer
    {
    public:
        static void SerializeMaterial(const String& path, Ref<Material>& mat);
        static void DeserializeMaterial(const String& path, Ref<Material>& mat);
        static void SerializePhysicsMaterial(const String& path, Ref<PhysicsMaterial>& pmat);
        static glm::vec3 DeserializePhysicsMaterial(const String& path);
    };
}
