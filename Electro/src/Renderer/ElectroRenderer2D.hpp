//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Electrobase.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroCamera.hpp"
#include "Renderer/ElectroEditorCamera.hpp"

namespace Electro::Renderer2D
{
    void Init();
    void Shutdown();

    void BeginScene(const Camera& camera, const glm::mat4& transform);
    void BeginScene(const EditorCamera& camera);
    void EndScene();

    void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
    void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
    void DrawDebugQuad(const glm::mat4& transform);
    void Flush();

    struct Statistics
    {
        Uint DrawCalls = 0;
        Uint QuadCount = 0;
        Uint GetTotalVertexCount() const { return QuadCount * 4; }
        Uint GetTotalIndexCount() const { return QuadCount * 6; }
    };

    void UpdateStats();
    Statistics GetStats();
    void StartBatch();
}
