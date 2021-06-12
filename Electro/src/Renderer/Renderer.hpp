//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Shadows.hpp"
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
        Ref<ConstantBuffer> TransformCBuffer;
        Ref<ConstantBuffer> LightSpaceMatrixCBuffer;

        // Draw Lists // TODO: Use a custom vector class for these draw lists
        Vector<DrawCommand> MeshDrawList;
        Vector<DrawCommand> OutlineDrawList;

        // Environment Map
        Ref<EnvironmentMap> EnvironmentMap;
        bool EnvironmentMapActivated = true;

        // Shadows
        Ref<Shader> ShadowMapShader;
        Electro::Shadows Shadows;

        Vector<Ref<Shader>> AllShaders;
        Vector<Ref<ConstantBuffer>> AllConstantBuffers;

        // Debug
        Ref<Shader> SolidColorShader;
        Ref<Shader> OutlineShader;

        Ref<Shader> GridShader;
        Ref<VertexBuffer> GridVertexBuffer;
        Ref<Pipeline> GridPipeline;

        Ref<Framebuffer> OutlineTexture;
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
        static void SubmitOutlineMesh(const Ref<Mesh>& mesh, const glm::mat4& transform);

        static void OnWindowResize(Uint width, Uint height);

        static void SetSceneContext(Scene* sceneContext) { sData->SceneContext = sceneContext; }
        static void SetActiveRenderBuffer(Ref<Framebuffer>& renderBuffer) { sData->ActiveRenderBuffer = renderBuffer; }

        static const Scope<RendererData>& GetData() { return sData; }
        static const Ref<Shader> GetShader(const String& nameWithoutExtension);
        static const Ref<ConstantBuffer> GetConstantBuffer(Uint index) { return sData->AllConstantBuffers[index]; }

        static Vector<Ref<Shader>>& GetAllShaders() { return sData->AllShaders; }
    private:
        static void ShadowPass();
        static void DebugPass();
        static void GeometryPass();
        static bool IsDrawListEmpty();
        static void ClearDrawList();
        static void RenderFullscreenQuad();
    private:
        static Scope<RendererData> sData;
    };
}
