//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/ElectroScene.hpp"
#include "Core/ElectroLog.hpp"
#include "Scene/ElectroComponents.hpp"
#include <entt.hpp>

namespace Electro
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            :m_EntityHandle(handle), m_Scene(scene) {}

        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            E_ASSERT(!HasComponent<T>(), "Entity already has this component!");
            T& component = m_Scene->mRegistry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            E_ASSERT(HasComponent<T>(), "Entity does not have this component!");
            return m_Scene->mRegistry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->mRegistry.has<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            m_Scene->mRegistry.remove_if_exists<T>(m_EntityHandle);
        }

        entt::entity Raw()
        {
            return m_EntityHandle;
        }

        bool IsValid()
        {
            return m_Scene->mRegistry.valid(m_EntityHandle);
        }

        void RemoveAllComponent()
        {
            m_Scene->mRegistry.remove_all(m_EntityHandle);
        }

        TransformComponent& Transform() { return m_Scene->mRegistry.get<TransformComponent>(m_EntityHandle); }
        const glm::mat4& Transform() const { return m_Scene->mRegistry.get<TransformComponent>(m_EntityHandle).GetTransform(); }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator Uint() const { return (Uint)m_EntityHandle; }

        bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
        bool operator!=(const Entity& other) const { return !(*this == other); }

        UUID GetUUID() { return (GetComponent<IDComponent>().ID); }
        UUID GetSceneUUID() { return m_Scene->GetUUID(); }
    public:
        Scene* m_Scene = nullptr;
    private:
        entt::entity m_EntityHandle{ entt::null };
        friend class Scene;
        friend class ScriptEngine;
    };
}