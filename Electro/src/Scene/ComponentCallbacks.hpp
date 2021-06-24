//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "SceneManager.hpp"
#include "Scripting/ScriptEngine.hpp"

namespace Electro
{
    static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = SceneManager::GetSceneMap()[sceneID];

        UUID entityID = registry.get<IDComponent>(entity).ID;
        EntityMap& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Entity doesn't exist!");
        ScriptEngine::InitScriptEntity(entityMap.at(entityID));
    }

    static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.back()).SceneID;

        if (registry.has<IDComponent>(entity))
        {
            UUID entityID = registry.get<IDComponent>(entity).ID;
            ScriptEngine::OnScriptComponentDestroyed(sceneID, entityID);
        }
    }

    static void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = SceneManager::GetSceneMap()[sceneID];

        if (registry.has<IDComponent>(entity))
        {
            CameraComponent& camera = registry.get<CameraComponent>(entity);
            const Pair<Uint, Uint>& viewportDimensions = scene->GetViewportDimensions();
            if (viewportDimensions.Data1 != 0 && viewportDimensions.Data2 != 0)
                camera.Camera.SetViewportSize(viewportDimensions.Data1, viewportDimensions.Data2);
        }
    }
}