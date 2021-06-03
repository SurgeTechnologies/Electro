//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Cascades.hpp"
#include "Mesh.hpp"

namespace Electro
{
    class Scene;
    struct DrawCommand
    {
        Ref<Electro::Mesh> Mesh;
        glm::mat4 Transform;
    };

    struct SceneRendererData
    {
        // Rendering Context
        Scene* SceneContext;
        Ref<Framebuffer> ActiveRenderBuffer;

        // Camera
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewMatrix;
        glm::mat4 ViewProjectionMatrix;

        // Constant Buffers
        Ref<ConstantBuffer> SceneCBuffer;
        Ref<ConstantBuffer> LightSpaceMatrixCBuffer;
        Ref<ConstantBuffer> CascadeEndsCBuffer;

        // Draw Lists // TODO: Use a custom vector class for these draw lists
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> ColliderDrawList;

        // Environment Map
        Ref<EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        // Shadows
        Ref<Shader> ShadowMapShader;
        Cascades ShadowMapCascades;

        // Status
        size_t DrawCalls = 0;
    };

    class SceneRenderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void BeginScene(EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void EndScene();

        static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform);
        static void SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform);
        static void SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform);
        static void SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform);
        static Ref<EnvironmentMap>& GetEnvironmentMapSlot();
        static bool& GetEnvironmentMapActivationBool();
        static void SetSceneContext(Scene* sceneContext);
        static void SetActiveRenderBuffer(Ref<Framebuffer>& renderBuffer);
        static const Scope<SceneRendererData>& GetData();
    private:
        static void ShadowPass();
        static void GeometryPass();
        static bool IsDrawListEmpty();
        static void ClearDrawList();
    };
}
