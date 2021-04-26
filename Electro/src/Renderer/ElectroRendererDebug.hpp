//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>
#include "Renderer/Camera/ElectroEditorCamera.hpp"
#include "Scene/ElectroSceneCamera.hpp"

namespace Electro
{
    class RendererDebug
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const EditorCamera& camera);
        static void EndScene();
        static void Flush();

        static void SubmitCameraFrustum(SceneCamera& camera, glm::mat4& transform, glm::vec3& pos);
        static void SubmitLine(glm::vec3& p1, glm::vec3& p2);
        static void RenderGrid();
    private:
        static void StartBatch();
        static void NextBatch();
    private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
        };

        static Scope<SceneData> mSceneData;
    };
}
