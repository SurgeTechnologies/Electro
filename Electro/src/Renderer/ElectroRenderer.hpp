//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroRenderCommand.hpp"
#include "Interface/ElectroConstantBuffer.hpp"
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
        static void DrawMesh(Ref<Mesh> mesh, const glm::mat4& transform);
        static void DrawColliderMesh(Ref<Mesh> mesh, const glm::mat4& transform);
        static void Submit(Ref<Pipeline> pipeline, Uint size);
        static RendererAPI::API GetAPI();

        static Uint GetTotalDrawCallsCount();
        static void UpdateStatus();
    };
}