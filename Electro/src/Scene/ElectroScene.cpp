//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroScene.hpp"
#include "Core/ElectroInput.hpp"
#include "Renderer/ElectroRenderer2D.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Scene/ElectroComponents.hpp"
#include "Scripting/ElectroScriptEngine.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"
#include "Physics/ElectroPhysicsActor.hpp"
#include "ElectroEntity.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    struct SceneComponent
    {
        UUID SceneID;
    };

    std::unordered_map<UUID, Scene*> sActiveScenes;

    static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = sActiveScenes[sceneID];

        auto entityID = registry.get<IDComponent>(entity).ID;
        E_ASSERT(scene->mEntityIDMap.find(entityID) != scene->mEntityIDMap.end(), "Entity already exists!");
        ScriptEngine::InitScriptEntity(scene->mEntityIDMap.at(entityID));
    }

    static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = sActiveScenes[sceneID];
        auto entityID = registry.get<IDComponent>(entity).ID;

        if(ScriptEngine::ModuleExists(registry.get<ScriptComponent>(entity).ModuleName))
            ScriptEngine::OnScriptComponentDestroyed(sceneID, entityID);
    }

    Scene::Scene(bool isRuntimeScene)
    {
        mRegistry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
        mRegistry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

        mRegistry.on_destroy<ScriptComponent>().disconnect<&OnScriptComponentConstruct>();
        mRegistry.on_destroy<ScriptComponent>().disconnect<&OnScriptComponentDestroy>();

        mSceneEntity = mRegistry.create();
        sActiveScenes[mSceneID] = this;
        mRegistry.emplace<SceneComponent>(mSceneEntity, mSceneID);

        if (isRuntimeScene)
            PhysicsEngine::CreateScene();
    }

    Scene::~Scene()
    {
        ScriptEngine::OnSceneDestruct(mSceneID);
        mRegistry.clear();
        sActiveScenes.erase(mSceneID);
        delete mLightningHandeler;
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
        if (entity.HasComponent<ScriptComponent>())
            ScriptEngine::OnScriptComponentDestroyed(mSceneID, entity.GetUUID());

        mRegistry.destroy(entity);
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
                Renderer2D::BeginScene(*mainCamera, cameraTransform);
                auto group = mRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                    if (sprite.Texture)
                        Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TilingFactor, sprite.Color);
                    else
                        Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
                }

                Renderer2D::EndScene();
            }
            {
                Renderer::BeginScene(*mainCamera, cameraTransform);
                PushLights();

                auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                    if (mesh.Mesh)
                    {
                        mLightningHandeler->CalculateAndRenderLights(cameraTransformComponent.Translation, mesh.Mesh->GetMaterial());
                        Renderer::SubmitMesh(mesh.Mesh, transform.GetTransform());
                    }
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
            Renderer2D::BeginScene(camera);

            auto group = mRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                if (sprite.Texture)
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TilingFactor, sprite.Color);
                else
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
            }

            Renderer2D::EndScene();
        }

        {
            Renderer::BeginScene(camera);
            PushLights();

            auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                if (mesh.Mesh)
                {
                    mLightningHandeler->CalculateAndRenderLights(camera.GetPosition(), mesh.Mesh->GetMaterial());
                    Renderer::SubmitMesh(mesh.Mesh, transform.GetTransform());
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

    void Scene::OnEvent(Event& e) { }

    void Scene::OnRuntimeStart()
    {
        ScriptEngine::SetSceneContext(this);
        {
            auto view = mRegistry.view<ScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::InstantiateEntityClass(e);
            }
        }

        {
            auto view = mRegistry.view<RigidBodyComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                PhysicsEngine::CreateActor(e);
            }
        }
        mIsPlaying = true;
    }

    void Scene::OnRuntimeStop()
    {
        PhysicsEngine::DestroyScene();
        mIsPlaying = false;
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
        CopyComponent<SpriteRendererComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<PointLightComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<SkyLightComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<ScriptComponent>(target->mRegistry, mRegistry, enttMap);

        CopyComponent<RigidBodyComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<PhysicsMaterialComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<BoxColliderComponent>(target->mRegistry, mRegistry, enttMap);
        CopyComponent<SphereColliderComponent>(target->mRegistry, mRegistry, enttMap);

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
        CopyComponentIfExists<SpriteRendererComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<PointLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<SkyLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<ScriptComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);

        CopyComponentIfExists<RigidBodyComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<PhysicsMaterialComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<BoxColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<SphereColliderComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);

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

    Ref<Scene> Scene::GetScene(UUID uuid)
    {
        if (sActiveScenes.find(uuid) != sActiveScenes.end())
            return sActiveScenes.at(uuid);

        return {};
    }

    void Scene::PushLights()
    {
        mLightningHandeler->ClearLights();
        {
            auto view = mRegistry.view<TransformComponent, SkyLightComponent>();
            for (auto entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, SkyLightComponent>(entity);
                mLightningHandeler->mSkyLights.push_back(SkyLight{ light.Color, light.Intensity });
            }
        }
        {
            auto view = mRegistry.view<TransformComponent, PointLightComponent>();
            for (auto entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                mLightningHandeler->mPointLights.push_back(PointLight{ transform.Translation, 0, light.Color, 0.0f, light.Intensity, light.Constant, light.Linear, light.Quadratic });
            }
        }
    }

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component) { static_assert(false); }

    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(mViewportWidth, mViewportHeight);
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<PhysicsMaterialComponent>(Entity entity, PhysicsMaterialComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
    {
    }
}