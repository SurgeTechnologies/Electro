//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroRenderCommand.hpp"
#include "ElectroEditorCamera.hpp"
#include "ElectroSkybox.hpp"
#include "ElectroConstantBuffer.hpp"
#include "ElectroMesh.hpp"

namespace Electro
{
    struct SceneCBufferData;
    struct SceneData;
    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void OnWindowResize(Uint width, Uint height);
        static void BeginScene(EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void EndScene();
        static bool& GetSkyboxActivationBool();
        static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform);
        static void Submit(Ref<Pipeline> pipeline, Uint size);
        static Ref<Skybox>& SetSkybox(const Ref<Skybox>& skybox);
        static void SetSkyboxActivationBool(bool vaule);

        static void UpdateStats();
        static size_t GetTotalDrawCallsCount();
        static RendererAPI::API GetAPI();
    };
}