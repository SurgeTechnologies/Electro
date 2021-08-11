//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Timer.hpp"
#include "Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Mesh.hpp"
#include "Lights.hpp"
#include "PostProcessing/Bloom.hpp"
#include "RenderPass/RenderPassManager.hpp"

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

    enum class ToneMappingAlgorithm : int
    {
        REINHARD       = 0,
        UNCHARTED2     = 1,
        REINHARD_JODIE = 2,
        ACES_APPROX    = 3,
        ACES_FITTED    = 4,
        DEFAULT = ACES_FITTED
    };

    struct CompositeParams
    {
        float Exposure = 1.0f;
        Electro::ToneMappingAlgorithm ToneMappingAlgorithm = ToneMappingAlgorithm::DEFAULT;
        glm::vec2 InverseScreenSize;

        int ApplyFXAA = 1;
        glm::ivec3 __Padding = { 0, 0, 0 };
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
        Electro::RenderPassManager RenderPassManager;

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
        Ref<ConstantBuffer> CompositeParamsCBuffer;

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

        // Composite Params
        Electro::CompositeParams CompositeParams;

        // All Shaders and ConstantBuffers
        Vector<Ref<Shader>> AllShaders;
        Vector<Ref<ConstantBuffer>> AllConstantBuffers;

        //Post Processing
        PostProcessingPipeline PostProcessPipeline;
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
        static void RenderFullscreenQuad();
    private:
        static void PostProcessing();
        static void CompositePass();

        static void ClearDrawList();
    private:
        static Scope<RendererData> sData;
    };
}
