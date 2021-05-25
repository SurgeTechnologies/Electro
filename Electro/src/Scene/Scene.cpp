//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Scene.hpp"
#include "Core/Input.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/SceneRenderer.hpp"
#include "Renderer/RendererDebug.hpp"
#include "Scene/Components.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "Physics/PhysicsActor.hpp"
#include "Entity.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    std::unordered_map<UUID, Scene*> sActiveScenes;
    //TODO: Move to SceneManager!
    struct SceneComponent
    {
        UUID SceneID;
    };

    static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = sActiveScenes[sceneID];
        auto entityID = registry.get<IDComponent>(entity).ID;
        E_ASSERT(scene->mEntityIDMap.find(entityID) != scene->mEntityIDMap.end(), "Entity doesn't exist!");
        ScriptEngine::InitScriptEntity(scene->mEntityIDMap.at(entityID));
    }

    static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        auto entityID = registry.get<IDComponent>(entity).ID;
        ScriptEngine::OnScriptComponentDestroyed(sceneID, entityID);
    }

    Scene::Scene(bool isRuntimeScene)
        : mIsRuntimeScene(isRuntimeScene), mLightningManager(new LightningManager())
    {
        mRegistry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
        mRegistry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

        mSceneEntity = mRegistry.create();
        mRegistry.emplace<SceneComponent>(mSceneEntity, mSceneID);
        sActiveScenes[mSceneID] = this;
    }

    Scene::~Scene()
    {
        mRegistry.on_destroy<ScriptComponent>().disconnect();
        ScriptEngine::OnSceneDestruct(mSceneID);
        mRegistry.clear();
        sActiveScenes.erase(mSceneID);
        delete mLightningManager;
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
                SceneRenderer::BeginScene(*mainCamera, cameraTransform);
                mLightningManager->ClearLights();
                {
                    {
                        auto view = mRegistry.view<TransformComponent, PointLightComponent>();
                        for (auto entity : view)
                        {
                            auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                            mLightningManager->PushPointLight(PointLight{ transform.Translation, light.Intensity, light.Color, 0.0f });
                        }
                    }
                    {
                        auto view = mRegistry.view<TransformComponent, DirectionalLightComponent>();
                        for (auto entity : view)
                        {
                            auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                            mLightningManager->PushDirectionalLight(DirectionalLight{ glm::normalize(transform.GetTransform()[2]), light.Intensity, light.Color, 0.0f });
                        }
                    }
                }

                auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
                for (auto entity : group)
                {
                    auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                    if (mesh.Mesh)
                    {
                        mLightningManager->CalculateAndRenderLights(cameraTransformComponent.Translation, mesh.Mesh->GetMaterials()[0]);
                        SceneRenderer::SubmitMesh(mesh.Mesh, transform.GetTransform());
                    }
                }
                SceneRenderer::EndScene();
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
            SceneRenderer::BeginScene(camera);
            mLightningManager->ClearLights();
            {
                {
                    auto view = mRegistry.view<TransformComponent, PointLightComponent>();
                    for (auto entity : view)
                    {
                        auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                        mLightningManager->PushPointLight(PointLight{ transform.Translation, light.Intensity, light.Color, 0.0f });
                    }
                }
                {
                    auto view = mRegistry.view<TransformComponent, DirectionalLightComponent>();
                    for (auto entity : view)
                    {
                        auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                        mLightningManager->PushDirectionalLight(DirectionalLight{ glm::normalize(transform.GetTransform()[2]), light.Intensity, light.Color, 0.0f });
                        //TODO: Batch these renderer debug calls
                        RendererDebug::BeginScene(camera);
                        RendererDebug::SubmitLine(transform.Translation, transform.GetTransform()[2]);
                        RendererDebug::EndScene();
                    }
                }
            }
            auto group = mRegistry.group<MeshComponent>(entt::get<TransformComponent>);
            for (auto entity : group)
            {
                auto [mesh, transform] = group.get<MeshComponent, TransformComponent>(entity);
                if (mesh.Mesh)
                {
                    mLightningManager->CalculateAndRenderLights(camera.GetPosition(), mesh.Mesh->GetMaterials()[0]);
                    SceneRenderer::SubmitMesh(mesh.Mesh, transform.GetTransform());
                    if (mSelectedEntity == entity)
                    {
                        RendererDebug::BeginScene(camera);
                        const Submesh& submesh = mesh.Mesh->GetSubmeshes()[0];
                        RendererDebug::DrawAABB(submesh.BoundingBox, transform.GetTransform() * submesh.Transform, { 0.9, 0.9, 0.1, 1.0f });
                        RendererDebug::EndScene();
                    }
                }
            }

            {
                auto view = mRegistry.view<BoxColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    auto& collider = e.GetComponent<BoxColliderComponent>();
                    if (mSelectedEntity == entity)
                        SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
                }
            }
            {
                auto view = mRegistry.view<SphereColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    auto& collider = e.GetComponent<SphereColliderComponent>();
                    if (mSelectedEntity == entity)
                        SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
                }
            }
            {
                auto view = mRegistry.view<MeshColliderComponent>();
                for (auto entity : view)
                {
                    Entity e = { entity, this };
                    auto& collider = e.GetComponent<MeshColliderComponent>();
                    if (mSelectedEntity == entity)
                        SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
                }
            }
            {
                RendererDebug::BeginScene(camera);
                RendererDebug::RenderGrid();
                {
                    auto view = mRegistry.view<TransformComponent, CameraComponent>();
                    for (auto entity : view)
                    {
                        auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                        if (camera.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
                            RendererDebug::SubmitCameraFrustum(camera.Camera, transform.GetTransform(), transform.Translation);
                    }
                }
                RendererDebug::EndScene();
            }

            SceneRenderer::EndScene();
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
        CopyComponent<SpriteRendererComponent>(target->mRegistry, mRegistry, enttMap);
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
        CopyComponentIfExists<SpriteRendererComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<PointLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<DirectionalLightComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        CopyComponentIfExists<ScriptComponent>(newEntity.mEntityHandle, entity.mEntityHandle, mRegistry);
        //Physics
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

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component) { static_assert(false); }
    #define ON_COMPOPNENT_ADDED_DEFAULT(x) template<> void Scene::OnComponentAdded<x>(Entity entity, x& component){}

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) { component.Camera.SetViewportSize(mViewportWidth, mViewportHeight); }
    ON_COMPOPNENT_ADDED_DEFAULT(IDComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(TransformComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(SpriteRendererComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(TagComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(MeshComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(PointLightComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(DirectionalLightComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(ScriptComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(RigidBodyComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(BoxColliderComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(SphereColliderComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(CapsuleColliderComponent)
    ON_COMPOPNENT_ADDED_DEFAULT(MeshColliderComponent)
}
