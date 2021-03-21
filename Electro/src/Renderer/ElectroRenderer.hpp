//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroRenderCommand.hpp"
#include "ElectroEditorCamera.hpp"
#include "ElectroSkybox.hpp"
#include "ElectroConstantBuffer.hpp"
#include "ElectroMesh.hpp"

namespace Electro::Renderer
{
    void Init();
    void Shutdown();
    void OnWindowResize(Uint width, Uint height);
    void BeginScene(EditorCamera& camera);
    void BeginScene(const Camera& camera, const glm::mat4& transform);
    void EndScene();

    void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform);
    void Submit(Ref<Pipeline> pipeline, Uint size);
    Ref<Skybox>& SetSkybox(const Ref<Skybox>& skybox);
    void SetSkyboxActivationBool(bool vaule);

    void UpdateStats();
    size_t GetTotalDrawCallsCount();

    RendererAPI::API GetAPI();
}