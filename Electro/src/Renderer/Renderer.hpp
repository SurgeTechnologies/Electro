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
        DrawCommand(const Ref<Mesh>& mesh, const glm::mat4& transform)
            : Mesh(mesh), Transform(transform) {}

        Ref<Electro::Mesh> Mesh;
        glm::mat4 Transform;
    };

    struct Line
    {
        glm::vec3 P1;
        glm::vec3 P2;
    };

    struct RendererData
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

        // Debug
        Ref<Shader> ColliderShader;
        Vector<Line> GridPositions;
        bool ShowGrid = true;
        bool ShowCameraFrustum = true;
        bool ShowAABB = false;
    };

    class Renderer
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
        static void OnWindowResize(Uint width, Uint height);
        static void SetSceneContext(Scene* sceneContext);
        static void SetActiveRenderBuffer(Ref<Framebuffer>& renderBuffer);
        static const Scope<RendererData>& GetData();

    private:
        static void ShadowPass();
        static void DebugPass();
        static void GeometryPass();
        static bool IsDrawListEmpty();
        static void ClearDrawList();
    };
}
