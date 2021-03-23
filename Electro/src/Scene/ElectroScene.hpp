//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Core/ElectroUUID.hpp"
#include "Scene/ElectroLightningHandeler.hpp"
#include "Renderer/ElectroEditorCamera.hpp"
#include "Core/ElectroTimestep.hpp"

#pragma warning(push, 0)
#include <entt.hpp>
#pragma warning(pop)

namespace Electro
{
    class Entity;
    using EntityMap = std::unordered_map<UUID, Entity>;

    class Scene : public IElectroRef
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const String& name = String());
        Entity CreateEntityWithID(UUID uuid, const String& name = "", bool runtimeMap = false);
        void DestroyEntity(Entity entity);
        void DuplicateEntity(Entity entity);
        void OnUpdate(Timestep ts);
        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        void OnViewportResize(Uint width, Uint height);
        void OnEvent(Event& e);

        void OnRuntimeStart();
        void OnRuntimeStop();
        void CopySceneTo(Ref<Scene>& target);

        UUID GetUUID() const { return m_SceneID; }
        static Ref<Scene> GetScene(UUID uuid);

        Entity GetPrimaryCameraEntity();
        Entity FindEntityByTag(const String& tag);
        EntityMap GetEntityMap() { return m_EntityIDMap; }

        template<typename T>
        auto GetAllEntitiesWith() { return m_Registry.view<T>(); }
    private:
        void PushLights();

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    public:
        EntityMap m_EntityIDMap;
        bool m_IsPlaying = false;

    private:
        UUID m_SceneID;
        Uint m_ViewportWidth = 0, m_ViewportHeight = 0;
        entt::entity m_SceneEntity;
        entt::registry m_Registry;

        LightningHandeler* m_LightningHandeler = new LightningHandeler();
        friend class Physics2D;
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };
}