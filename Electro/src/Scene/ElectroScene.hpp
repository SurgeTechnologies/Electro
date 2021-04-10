//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Core/ElectroUUID.hpp"
#include "Core/ElectroTimestep.hpp"
#include "Scene/ElectroLightningHandeler.hpp"
#include "Renderer/ElectroEditorCamera.hpp"
#pragma warning(push, 0)
#include <entt.hpp>
#pragma warning(pop)

namespace Electro
{
    class Entity;
    class PhysicsSceneSlot;
    using EntityMap = std::unordered_map<UUID, Entity>;

    class Scene : public IElectroRef
    {
    public:
        Scene(bool isRuntimeScene = false);
        ~Scene();

        Entity CreateEntity(const String& name = String());
        Entity CreateEntityWithID(UUID uuid, const String& name = "", bool runtimeMap = false);
        void DestroyEntity(Entity entity);
        void DuplicateEntity(Entity entity);
        void OnUpdate(Timestep ts);
        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        void OnViewportResize(Uint width, Uint height);

        void OnRuntimeStart();
        void OnRuntimeStop();
        void CopySceneTo(Ref<Scene>& target);

        UUID GetUUID() const { return mSceneID; }
        Entity GetPrimaryCameraEntity();
        Entity FindEntityByTag(const String& tag);
        EntityMap GetEntityMap() { return mEntityIDMap; }

        template<typename T>
        auto GetAllEntitiesWith() { return mRegistry.view<T>(); }
    private:
        void PushLights();

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);
    public:
        entt::entity mSelectedEntity;
        EntityMap mEntityIDMap;
        bool mIsPlaying = false;
        bool mIsRuntimeScene;
    private:
        UUID mSceneID;
        Uint mViewportWidth = 0, mViewportHeight = 0;
        entt::entity mSceneEntity;
        entt::registry mRegistry;

        LightningManager* mLightningManager = nullptr;
        PhysicsSceneSlot* mPhysicsSceneSlot = nullptr;
        friend class Physics2D;
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class SceneManager;
    };
}