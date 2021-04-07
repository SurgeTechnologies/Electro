//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroSceneManager.hpp"
#include "ElectroEntity.hpp"
#include "ElectroScene.hpp"

namespace Electro
{
    std::unordered_map<Electro::UUID, Scene*> SceneManager::sActiveScenes;
    std::unordered_map<Electro::UUID, Scene*>& SceneManager::GetActiveScenes()
    {
        return sActiveScenes;
    }

    void SceneManager::PushWorkerScene(Scene* scene)
    {
        if (sActiveScenes.size() >= 2)
        {
            ELECTRO_CRITICAL("Scene Manager's scene container is full, you can't have more than 2 scenes! Call to PushScene(...) aborted.");
            return;
        }
        sActiveScenes[scene->mSceneID] = scene;
        scene->mRegistry.emplace<SceneComponent>(scene->mSceneEntity, scene->mSceneID);
    }

    void SceneManager::EraseScene(UUID sceneID)
    {
        sActiveScenes.erase(sceneID);
    }

    Ref<Scene> SceneManager::GetScene(UUID uuid)
    {
        if (sActiveScenes.find(uuid) != sActiveScenes.end())
            return sActiveScenes.at(uuid);

        ELECTRO_WARN("Requested scene not found!");
        return {};
    }

    Ref<Scene> SceneManager::GetRuntimeScene()
    {
        for (auto& scene : sActiveScenes)
        {
            if (scene.second->mIsRuntimeScene)
                return scene.second;
        }
        ELECTRO_WARN("Runtime scene not found!");
        return {};
    }
}
