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
        Scene(const String& debugName, bool isRuntimeScene = false);
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
        const String& GetName() const { return mName; }
        entt::entity GetSelectedEntity() const { return mSelectedEntity; }
        void SetSelectedEntity(entt::entity e) { mSelectedEntity = e; }

        const entt::registry& GetRegistry() const { return mRegistry; }
        entt::registry& GetRegistry() { return mRegistry; }

        bool IsRuntimeScene() { return mIsRuntimeScene; }
        bool IsPlaying() { return mIsPlaying; }

        Entity GetPrimaryCameraEntity();
        Entity FindEntityByTag(const String& tag);
        EntityMap GetEntityMap() const { return mEntityIDMap; }
        Pair<Uint, Uint> GetViewportDimensions() const { return { mViewportWidth, mViewportHeight }; }
        bool EntityExists(uint64_t entityID);

        template<typename T>
        auto GetAllEntitiesWith() { return mRegistry.view<T>(); }
    private:
        UUID mSceneID;
        entt::entity mSceneEntity;
        entt::entity mSelectedEntity = {};
        entt::registry mRegistry;
        bool mIsRuntimeScene;
        bool mIsPlaying = false;
        EntityMap mEntityIDMap;
        Uint mViewportWidth = 0, mViewportHeight = 0;
        String mName;
        PhysicsSceneSlot* mPhysicsSceneSlot = nullptr;

        friend class SceneSerializer;
    };
}
