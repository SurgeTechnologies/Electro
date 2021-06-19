//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Scene.hpp"
#include "Core/Log.hpp"
#include "Scene/Components.hpp"
#include <entt.hpp>

namespace Electro
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            : mEntityHandle(handle), mScene(scene) {}

        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            E_ASSERT(!HasComponent<T>(), "Entity already has this component!");
            T& component = mScene->GetRegistry().emplace<T>(mEntityHandle, std::forward<Args>(args)...);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            E_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            return mScene->GetRegistry().get<T>(mEntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return mScene->GetRegistry().has<T>(mEntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            mScene->GetRegistry().remove_if_exists<T>(mEntityHandle);
        }

        entt::entity Raw()
        {
            return mEntityHandle;
        }

        bool IsValid()
        {
            return mScene->GetRegistry().valid(mEntityHandle);
        }

        void RemoveAllComponent()
        {
            mScene->GetRegistry().remove_all(mEntityHandle);
        }

        TransformComponent& Transform() { return mScene->GetRegistry().get<TransformComponent>(mEntityHandle); }
        const glm::mat4& Transform() const { return mScene->GetRegistry().get<TransformComponent>(mEntityHandle).GetTransform(); }

        operator bool() const { return mEntityHandle != entt::null; }
        operator entt::entity() const { return mEntityHandle; }
        operator Uint() const { return (Uint)mEntityHandle; }

        bool operator==(const Entity& other) const { return mEntityHandle == other.mEntityHandle && mScene == other.mScene; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

        UUID GetUUID() { return (GetComponent<IDComponent>().ID); }
        UUID GetSceneUUID() { return mScene->GetUUID(); }
    public:
        Scene* mScene = nullptr;
    private:
        entt::entity mEntityHandle{ entt::null };
        friend class Scene;
        friend class ScriptEngine;
    };
}
