//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "SceneManager.hpp"
#include <fmt/color.h>
#include <fmt/os.h>

namespace Electro
{
    std::unordered_map<UUID, Scene*> sActiveScenes;

    void SceneManager::PushScene(UUID sceneID, Scene* scene)
    {
        sActiveScenes[sceneID] = scene;
    }

    void SceneManager::EraseScene(UUID sceneID)
    {
        sActiveScenes.erase(sceneID);
    }

    Ref<Scene> SceneManager::GetScene(UUID sceneID)
    {
        if (sActiveScenes.find(sceneID) != sActiveScenes.end())
            return sActiveScenes.at(sceneID);

        return {};
    }

    std::unordered_map<Electro::UUID, Scene*>& SceneManager::GetSceneMap()
    {
        return sActiveScenes;
    }
}