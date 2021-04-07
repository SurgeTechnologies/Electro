//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Core/ElectroUUID.hpp"

namespace Electro
{
    class Scene;
    struct SceneComponent
    {
        UUID SceneID;
    };

    class SceneManager
    {
    public:
        static std::unordered_map<UUID, Scene*>& GetActiveScenes();
        static void PushWorkerScene(Scene* scene);
        static void EraseScene(UUID sceneID);
        static Ref<Scene> GetScene(UUID uuid);
        static Ref<Scene> GetRuntimeScene();
    private:
        static std::unordered_map<UUID, Scene*> sActiveScenes;
    };
}
