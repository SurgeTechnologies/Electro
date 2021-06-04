//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Math/BoundingBox.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Scene/Components.hpp"
#include "Scene/SceneCamera.hpp"

namespace Electro
{
    class RendererDebug
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const glm::mat4& viewProjection);
        static void EndScene();
        static void Flush();

        static void SubmitCameraFrustum(SceneCamera& camera, const glm::mat4& transform);
        static void SubmitCameraFrustum(glm::vec4* points, const glm::mat4& transform, const glm::vec4& color);
        static void SubmitCameraFrustum(glm::vec3* points, const glm::vec4& color);
        static void SubmitLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
        static void DrawAABB(const BoundingBox& aabb, const glm::mat4& transform, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
        static void DrawAABB(glm::vec4* corners, const glm::mat4& transform, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f });
        static void RenderGrid();
        static void SubmitOrthogonalProjection(const glm::mat4& viewProjection);

        static Pair<bool*, bool*> GetToggles();
    private:
        static void StartBatch();
        static void NextBatch();
    };
}
