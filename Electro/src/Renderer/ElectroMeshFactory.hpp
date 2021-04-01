//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Renderer/ElectroMesh.hpp"

namespace Electro
{
    class MeshFactory
    {
    public:
        static Ref<Mesh> CreateCube(const glm::vec3& size);
        static Ref<Mesh> CreateSphere(float radius);
        static Ref<Mesh> CreateCapsule(float radius, float height);
    };
}
