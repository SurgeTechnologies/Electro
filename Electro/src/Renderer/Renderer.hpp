//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Timer.hpp"
#include "Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Shadows.hpp"
#include "Mesh.hpp"
#include "Lights.hpp"

#define GAUSSIAN_RADIUS 7

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

    struct ColliderDrawCommand
    {
        ColliderDrawCommand(const Ref<Mesh>& mesh, const glm::mat4& transform, bool show)
            : Mesh(mesh), Transform(transform), Show(show) {}

        Ref<Electro::Mesh> Mesh;
        glm::mat4 Transform;
        bool Show = false;
    };

    enum class RendererBackend
    {
        DirectX11 = 0,
        OpenGL,
    };

    struct BlurParams
    {
        float Coefficients[GAUSSIAN_RADIUS + 1];
        int Radius;
        int Direction;

        int __Padding[2] = {};
    };

    struct RendererData
    {
        // Rendering Context
        Scene* SceneContext;
        Ref<Framebuffer> FinalColorBuffer;

        // Camera
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewMatrix;
        glm::mat4 ViewProjectionMatrix;
        glm::vec3 CameraPosition;

        // Constant Buffers
        Ref<ConstantBuffer> SceneCBuffer;
        Ref<ConstantBuffer> TransformCBuffer;
        Ref<ConstantBuffer> LightSpaceMatrixCBuffer;
        Ref<ConstantBuffer> InverseViewProjectionCBuffer;
        Ref<ConstantBuffer> LightConstantBuffer;
        Ref<ConstantBuffer> SolidColorCBuffer;

        // Draw Lists // TODO: Use a custom vector class for these draw lists
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> OutlineDrawList;
        Vector<ColliderDrawCommand> ColliderDrawList;

        // Environment Map
        Ref<EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        // Lights
        LightCBuffer LightCBufferData;
        Vector<PointLight> AllPointLights;
        Vector<DirectionalLight> AllDirectionalLights;

        // Bloom
        Electro::BlurParams BlurParams;
        Ref<Framebuffer> BloomRenderTargets[2];
        Ref<Shader> ThresholdDownsampleShader;
        Ref<Shader> GaussianBlurShader;
        Ref<Shader> QuadCompositeShader;
        Ref<ConstantBuffer> BloomThresholdCBuffer;
        Ref<ConstantBuffer> BlurParamsCBuffer;
        bool BloomEnabled = true;
        float BloomThreshold = 1.1f;

        // Shadows
        Ref<Shader> ShadowMapShader;
        Electro::Shadows Shadows;
        bool ShadowsEnabled = true;

        // All Shaders and ConstantBuffers
        Vector<Ref<Shader>> AllShaders;
        Vector<Ref<ConstantBuffer>> AllConstantBuffers;

        // Debug
        Ref<Shader> SolidColorShader;
        Ref<Shader> OutlineShader;
        Ref<Shader> GridShader;

        Ref<Framebuffer> OutlineRenderBuffer;

        bool ShowGrid = true;
        bool ShowCameraFrustum = true;
        bool ShowAABB = false;
        Uint TotalDrawCalls = 0;
    private:
        Electro::RendererBackend RendererBackend;
        friend class Renderer;
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
        static void SubmitOutlineMesh(const Ref<Mesh>& mesh, const glm::mat4& transform);

        static void SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform, bool show);

        static void SubmitPointLight(const PointLight& pointLight);
        static void SubmitDirectionalLight(const DirectionalLight& directionalLight);

        static void OnWindowResize(Uint width, Uint height);
        static void CalculateAndRenderLights(const glm::vec3& cameraPos);

        static void SetSceneContext(Scene* sceneContext) { sData->SceneContext = sceneContext; }

        static const Scope<RendererData>& GetData() { return sData; }
        static const Ref<Shader> GetShader(const String& nameWithoutExtension);
        static const Ref<ConstantBuffer> GetConstantBuffer(Uint index) { return sData->AllConstantBuffers[index]; }
        static Ref<Framebuffer> GetFinalPassTexture() { return sData->FinalColorBuffer; }

        static Vector<Ref<Shader>>& GetAllShaders() { return sData->AllShaders; }
        static const RendererBackend GetBackend() { return sData->RendererBackend; }
        static void SetBackend(const RendererBackend backend) { sData->RendererBackend = backend; }
    private:
        static void ShadowPass();
        static void DebugPass();
        static void GeometryPass();
        static void BloomPass();
        static void CompositePass();

        static bool IsDrawListEmpty();
        static void ClearDrawList();
        static void RenderFullscreenQuad();
        static void ClearLights();
    private:
        static Scope<RendererData> sData;
    };
}
