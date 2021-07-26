//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Timer.hpp"
#include "Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Shadows.hpp"
#include "Mesh.hpp"
#include "Lights.hpp"
#include "PostProcessing/Bloom.hpp"

namespace Electro
{
    class Scene;
    struct DrawCommand
    {
        DrawCommand(const MeshComponent& meshComponent, const glm::mat4& transform)
            : MeshComp(meshComponent), Transform(transform) {}

        const Ref<Mesh>& GetMesh() const { return MeshComp.Mesh; }
        const glm::mat4& GetTransform() const { return Transform; }
        const MeshComponent& GetMeshComponent() const { return MeshComp; }
    private:
        MeshComponent MeshComp;
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

    struct RendererData
    {
        // Rendering Context
        Scene* SceneContext;
        bool RenderToSwapChain = false;
        Ref<Renderbuffer> GeometryBuffer;
        Ref<Renderbuffer> FinalSceneBuffer;

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
        Ref<ConstantBuffer> ExposureCBuffer;

        // Draw Lists // TODO: Use a custom vector class for these draw lists
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> OutlineDrawList;
        Vector<ColliderDrawCommand> ColliderDrawList;

        // Environment Map
        Ref<Electro::EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        // Lights
        LightCBuffer LightCBufferData;
        Vector<PointLight> AllPointLights;
        Vector<DirectionalLight> AllDirectionalLights;

        // Bloom
        Ref<Shader> QuadCompositeShader;
        float Exposure = 1.0f;

        // Shadows
        Ref<Shader> ShadowMapShader;
        Electro::Shadows Shadows;
        bool ShadowsEnabled = true;

        // All Shaders and ConstantBuffers
        Vector<Ref<Shader>> AllShaders;
        Vector<Ref<ConstantBuffer>> AllConstantBuffers;

        //Post Processing
        PostProcessingPipeline PostProcessPipeline;

        // Debug
        Ref<Shader> SolidColorShader;
        Ref<Shader> OutlineShader;
        Ref<Shader> GridShader;

        Ref<Renderbuffer> OutlineRenderBuffer;

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

        static void SubmitMesh(const MeshComponent& mesh, const glm::mat4& transform);
        static void SubmitOutlineMesh(const MeshComponent& mesh, const glm::mat4& transform);

        static void SubmitColliderMesh(const BoxColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const SphereColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const CapsuleColliderComponent& component, const glm::mat4& transform, bool show);
        static void SubmitColliderMesh(const MeshColliderComponent& component, const glm::mat4& transform, bool show);

        static void SubmitPointLight(const PointLight& pointLight);
        static void SubmitDirectionalLight(const DirectionalLight& directionalLight);

        static void OnWindowResize(Uint width, Uint height);
        static void CalculateAndRenderLights(const glm::vec3& cameraPos);

        static void SetSceneContext(Scene* sceneContext) { sData->SceneContext = sceneContext; }
        static void RenderToSwapchain();

        static const Scope<RendererData>& GetData() { return sData; }
        static const Ref<Shader> GetShader(const String& nameWithoutExtension);
        static const Ref<ConstantBuffer> GetConstantBuffer(Uint index) { return sData->AllConstantBuffers[index]; }

        static Ref<Renderbuffer>& GetFinalPassTexture() { return sData->FinalSceneBuffer; }
        static const Ref<Renderbuffer>& GetBloomBlurTexture() { return sData->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY)->GetOutputRenderBuffer(); }

        static Vector<Ref<Shader>>& GetAllShaders() { return sData->AllShaders; }
        static const RendererBackend GetBackend() { return sData->RendererBackend; }
        static void SetBackend(const RendererBackend backend) { sData->RendererBackend = backend; }
    private:
        static void ShadowPass();
        static void DebugPass();
        static void GeometryPass();
        static void PostProcessing();
        static void CompositePass();

        static bool IsDrawListEmpty();
        static void ClearDrawList();
        static void RenderFullscreenQuad();
        static void ClearLights();
    private:
        static Scope<RendererData> sData;
    };
}
