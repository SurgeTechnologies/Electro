//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Electrobase.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/Camera/ElectroCamera.hpp"
#include "Renderer/Camera/ElectroEditorCamera.hpp"

namespace Electro
{
    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const EditorCamera& camera);
        static void EndScene();
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawDebugQuad(const glm::mat4& transform);
        static void Flush();

        struct Statistics
        {
            Uint DrawCalls = 0;
            Uint QuadCount = 0;
            Uint GetTotalVertexCount() const { return QuadCount * 4; }
            Uint GetTotalIndexCount() const { return QuadCount * 6; }
        };

        static void UpdateStats();
        static Statistics GetStats();
        static void StartBatch();
    };
}
