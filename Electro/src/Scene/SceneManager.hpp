//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Entity.hpp"
#include "Scene.hpp"

namespace Electro
{
    struct SceneComponent
    {
        UUID SceneID;
    };

    class SceneManager
    {
    public:
        static void PushScene(UUID sceneID, Scene* scene);
        static void EraseScene(UUID sceneID);
        static Ref<Scene> GetScene(UUID sceneID);
        static std::unordered_map<UUID, Scene*>& GetSceneMap();
    };
}