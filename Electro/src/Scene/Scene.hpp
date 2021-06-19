//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/UUID.hpp"
#include "Core/Timestep.hpp"
#include "Renderer/Camera/EditorCamera.hpp"

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
        const entt::registry& GetRegistry() const { return mRegistry; }
        entt::registry& GetRegistry() { return mRegistry; }

        Entity GetPrimaryCameraEntity();
        Entity FindEntityByTag(const String& tag);
        bool EntityExists(uint64_t entityID);
        EntityMap GetEntityMap() const { return mEntityIDMap; }
        static Ref<Scene> GetScene(UUID sceneID);

        template<typename T>
        auto GetAllEntitiesWith() { return mRegistry.view<T>(); }
    public:
        entt::entity mSelectedEntity = {};
        EntityMap mEntityIDMap;
        bool mIsPlaying = false;
        bool mIsRuntimeScene;
    private:
        UUID mSceneID;
        Uint mViewportWidth = 0, mViewportHeight = 0;
        entt::entity mSceneEntity;
        entt::registry mRegistry;

        PhysicsSceneSlot* mPhysicsSceneSlot = nullptr;
    };
}
