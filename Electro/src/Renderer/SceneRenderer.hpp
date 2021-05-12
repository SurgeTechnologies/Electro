//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Camera/EditorCamera.hpp"
#include "Mesh.hpp"
#include "Scene/Components.hpp"

namespace Electro
{
    class SceneRenderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void BeginScene(EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void EndScene();

        static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform);
        static void SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform);
        static void SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform);
        static void SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform);

        static Ref<EnvironmentMap>& GetEnvironmentMapSlot();
        static bool& GetEnvironmentMapActivationBool();
    };
}