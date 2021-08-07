//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Scene.hpp"
#include "ComponentCallbacks.hpp"
#include "Renderer/Renderer.hpp"
#include "Physics/PhysicsActor.hpp"

namespace Electro
{
    Scene::Scene(const String& debugName, bool isRuntimeScene)
        : mIsRuntimeScene(isRuntimeScene), mName(debugName)
    {
        mRegistry.on_construct<CameraComponent>().connect<&OnCameraComponentConstruct>();
        mRegistry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
        mRegistry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

        mSceneEntity = mRegistry.create();
        mRegistry.emplace<SceneComponent>(mSceneEntity, mSceneID);
        SceneManager::PushScene(mSceneID, this);
    }

    Scene::~Scene()
    {
        mRegistry.on_destroy<CameraComponent>().disconnect();
        mRegistry.on_destroy<ScriptComponent>().disconnect();
        mRegistry.clear();
        SceneManager::EraseScene(mSceneID);
        ScriptEngine::OnSceneDestruct(mSceneID);
    }

    Entity Scene::CreateEntity(const String& name)
    {
        auto entity = Entity{ mRegistry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID = {};

        entity.AddComponent<TransformComponent>();
        if (!name.empty())
            entity.AddComponent<TagComponent>(name);

        mEntityIDMap[idComponent.ID] = entity;
        return entity;
    }

    Entity Scene::CreateEntityWithID(UUID uuid, const String& name, bool runtimeMap)
    {
        auto entity = Entity{ mRegistry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID = uuid;

        entity.AddComponent<TransformComponent>();
        if (!name.empty())
            entity.AddComponent<TagComponent>(name);

        E_ASSERT(mEntityIDMap.find(uuid) == mEntityIDMap.end(), "Entity with the given id already exists!");
        mEntityIDMap[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        if (ScriptEngine::IsEntityModuleValid(entity))
            ScriptEngine::OnScriptComponentDestroyed(mSceneID, entity.GetUUID());

        mRegistry.destroy(entity);
    }

    void Scene::OnRuntimeStart()
    {
        mPhysicsSceneSlot = new PhysicsSceneSlot();
        mPhysicsSceneSlot->CreateScene();

        ScriptEngine::SetSceneContext(this);

        // Create all the Physics Actors
        {
            auto view = mRegistry.view<RigidBodyComponent>();
            for (auto& entity : view)
            {
                Entity e = { entity, this };
                PhysicsEngine::CreateActor(e);
            }
        }

        // Instantiate all the script classes
        {
            auto view = mRegistry.view<ScriptComponent>();
            for (auto& entity : view)
            {
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::InstantiateEntityClass(e);
            }
        }

        mIsPlaying = true;
    }

    void Scene::OnRuntimeStop()
    {
        mPhysicsSceneSlot->DestroyScene();
        delete mPhysicsSceneSlot;
        mIsPlaying = false;
    }

    void Scene::OnUpdate(Timestep ts)
    {
        PhysicsEngine::Simulate(ts);
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        Camera* mainCamera = nullptr;
        TransformComponent cameraTransformComponent;
        glm::mat4 cameraTransform;

        {
            auto view = mRegistry.view<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    cameraTransformComponent = transform;
                    break;
                }
            }
        }
        if (mainCamera)
        {
            {
                Renderer::BeginScene(*mainCamera, cameraTransform);
                {
                    {
                        auto view = mRegistry.view<TransformComponent, PointLightComponent>();
                        for (auto entity : view)
                        {
                            auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                            Renderer::SubmitPointLight(PointLight{ transform.Translation, light.Intensity, light.Color, light.Radius });
                        }
                    }
                    {
                        auto view = mRegistry.view<TransformComponent, DirectionalLightComponent>();
                        for (auto entity : view)
                        {
                            auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                            Renderer::SubmitDirectionalLight(DirectionalLight{ glm::normalize(transform.GetTransform()[2]), light.Intensity, light.Color, 0.0f });
                        }
                    }
                }

                auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                    if (mesh.Mesh)
                        Renderer::SubmitMesh(mesh, transform.GetTransform());
                }
                Renderer::EndScene();
            }
        }

        {
            auto view = mRegistry.view<ScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::OnUpdate(e, ts);
            }
        }
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        {
            Renderer::BeginScene(camera);
            {
                {
                    auto view = mRegistry.view<TransformComponent, PointLightComponent>();
                    for (auto entity : view)
                    {
                        auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                        Renderer::SubmitPointLight(PointLight{ transform.Translation, light.Intensity, light.Color, light.Radius });
                    }
                }
                {
                    auto view = mRegistry.view<TransformComponent, DirectionalLightComponent>();
                    for (auto entity : view)
                    {
                        auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                        Renderer::SubmitDirectionalLight(DirectionalLight{ glm::normalize(transform.GetTransform()[2]), light.Intensity, light.Color, 0.0f });
                    }
                }
            }
            {
                auto view = mRegistry.view<BoxColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    BoxColliderComponent& collider = e.GetComponent<BoxColliderComponent>();
                    if (mSelectedEntity == entity)
                        Renderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform(), collider.ShowColliderBounds);
                }
            }
            {
                auto view = mRegistry.view<SphereColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    SphereColliderComponent& collider = e.GetComponent<SphereColliderComponent>();
                    if (mSelectedEntity == entity)
                        Renderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform(), collider.ShowColliderBounds);
                }
            }
            {
                auto view = mRegistry.view<CapsuleColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    CapsuleColliderComponent& collider = e.GetComponent<CapsuleColliderComponent>();
                    if (mSelectedEntity == entity)
                        Renderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform(), collider.ShowColliderBounds);
                }
            }
            {
                auto view = mRegistry.view<MeshColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    MeshColliderComponent& collider = e.GetComponent<MeshColliderComponent>();
                    if (mSelectedEntity == entity)
                        Renderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform(), collider.ShowColliderBounds);
                }
            }
            auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                if (mesh.Mesh)
                {
                    Renderer::SubmitMesh(mesh, transform.GetTransform());
                    if (entity == mSelectedEntity)
                        Renderer::SubmitOutlineMesh(mesh, transform.GetTransform());
                }
            }
            Renderer::EndScene();
        }
    }

    template<typename T>
    static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        auto components = srcRegistry.view<T>();
        for (auto srcEntity : components)
        {
            entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

            auto& srcComponent = srcRegistry.get<T>(srcEntity);
            auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
        }
    }

    void Scene::OnViewportResize(Uint width, Uint height)
    {
        mViewportWidth = width;
        mViewportHeight = height;

        auto view = mRegistry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }
    }

    void Scene::CopySceneTo(Ref<Scene>& target)
    {
        std::unordered_map<UUID, entt::entity> enttMap;
        auto idComponents = mRegistry.view<IDComponent>();
        for (auto entity : idComponents)
        {
            auto uuid = mRegistry.get<IDComponent>(entity).ID;
            Entity e = target->CreateEntityWithID(uuid, "", true);
            enttMap[uuid] = e.Raw();
        }

        CopyComponent<TagComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<TransformComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<MeshComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<CameraComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<PointLightComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<DirectionalLightComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<ScriptComponent>(target->mRegistry, mRegistry, enttMap);
        //Physics
        CopyComponent<RigidBodyComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<BoxColliderComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<SphereColliderComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<CapsuleColliderComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<MeshColliderComponent>(target->mRegistry, mRegistry, enttMap);

        auto& entityInstanceMap = ScriptEngine::GetEntityInstanceMap();
        if (entityInstanceMap.find(target->GetUUID()) != entityInstanceMap.end())
            ScriptEngine::CopyEntityScriptData(target->GetUUID(), mSceneID);
    }

    template<typename T>
    static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
    {
        if (registry.has<T>(src))
        {
            auto& srcComponent = registry.get<T>(src);
            registry.emplace_or_replace<T>(dst, srcComponent);
        }
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity;
        if (entity.HasComponent<TagComponent>())
            newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
        else
            newEntity = CreateEntity();

        CopyComponentIfExists<TransformComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<MeshComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<CameraComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<PointLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<DirectionalLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<ScriptComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        // Physics
        CopyComponentIfExists<RigidBodyComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<BoxColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<SphereColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<CapsuleColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<MeshColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = mRegistry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
                return Entity{ entity, this };
        }
        return {};
    }

    Entity Scene::FindEntityByTag(const String& tag)
    {
        auto view = mRegistry.view<TagComponent>();
        for (auto entity : view)
        {
            const auto& theRealTag = view.get<TagComponent>(entity).Tag;
            if (theRealTag == tag)
                return Entity{ entity, this };
        }
        return {};
    }

    bool Scene::EntityExists(uint64_t entityID)
    {
        auto view = mRegistry.view<IDComponent>();
        for (auto entity : view)
        {
            const auto& theRealID = view.get<IDComponent>(entity).ID;
            if (theRealID == entityID)
                return true;
        }
        return false;
    }
}
