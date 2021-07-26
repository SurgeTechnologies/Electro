//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "SceneSerializer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Asset/AssetManager.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "Physics/PhysXInternal.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Utility/YamlHelpers.hpp"

namespace Electro
{
    static bool CheckPath(const String& path)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (f)
            fclose(f);
        if (f != nullptr)
            return true;

        Log::Error("Cannot open path {0}", path);
        return false;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : mScene(scene) {}

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        if (entity.HasComponent<IDComponent>())
        {
            UUID uuid = entity.GetComponent<IDComponent>().ID;
            out << YAML::BeginMap; // Entity
            out << YAML::Key << "Entity" << YAML::Value << uuid;

            if (entity.HasComponent<TagComponent>())
            {
                out << YAML::Key << "TagComponent";
                out << YAML::BeginMap; // TagComponent

                auto& tag = entity.GetComponent<TagComponent>().Tag;
                out << YAML::Key << "Tag" << YAML::Value << tag;

                out << YAML::EndMap; // TagComponent
            }

            if (entity.HasComponent<TransformComponent>())
            {
                out << YAML::Key << "TransformComponent";
                out << YAML::BeginMap; // TransformComponent

                auto& tc = entity.GetComponent<TransformComponent>();
                out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
                out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
                out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

                out << YAML::EndMap; // TransformComponent
            }

            if (entity.HasComponent<CameraComponent>())
            {
                out << YAML::Key << "CameraComponent";
                out << YAML::BeginMap; // CameraComponent

                auto& cameraComponent = entity.GetComponent<CameraComponent>();
                auto& camera = cameraComponent.Camera;

                out << YAML::Key << "Camera" << YAML::Value;
                out << YAML::BeginMap; // Camera
                out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
                out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
                out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
                out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
                out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
                out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
                out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
                out << YAML::EndMap; // Camera

                out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
                out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

                out << YAML::EndMap; // CameraComponent
            }

            if (entity.HasComponent<MeshComponent>())
            {
                out << YAML::Key << "MeshComponent";
                out << YAML::BeginMap; // MeshComponent

                MeshComponent meshComponent = entity.GetComponent<MeshComponent>();
                out << YAML::Key << "AssetPath" << YAML::Value << AssetManager::GetRelativePath(meshComponent.Mesh->GetPath());
                out << YAML::Key << "CastShadows" << YAML::Value << meshComponent.CastShadows;

                // Serialize the materials
                Vector<Ref<Material>>& materials = meshComponent.Mesh->GetMaterials();
                size_t totalMaterials = materials.size();
                out << YAML::Key << "Total Materials" << YAML::Value << totalMaterials;
                String key = "Material";
                for (Uint i = 0; i < totalMaterials; i++)
                {
                    Ref<Material>& mat = materials[i];
                    out << YAML::Key << fmt::format("{0}-{1}", key, i) << mat->GetHandle();
                }
                out << YAML::EndMap; // MeshComponent
            }

            if (entity.HasComponent<PointLightComponent>())
            {
                out << YAML::Key << "PointLightComponent";
                out << YAML::BeginMap; // PointLightComponent

                auto& pointLight = entity.GetComponent<PointLightComponent>();
                out << YAML::Key << "Color" << YAML::Value << pointLight.Color;
                out << YAML::Key << "Intensity" << YAML::Value << pointLight.Intensity;
                out << YAML::Key << "Radius" << YAML::Value << pointLight.Radius;

                out << YAML::EndMap; // PointLightComponent
            }

            if (entity.HasComponent<DirectionalLightComponent>())
            {
                out << YAML::Key << "DirectionalLightComponent";
                out << YAML::BeginMap; // DirectionalLightComponent

                auto& dirLight = entity.GetComponent<DirectionalLightComponent>();
                out << YAML::Key << "Color" << YAML::Value << dirLight.Color;
                out << YAML::Key << "Intensity" << YAML::Value << dirLight.Intensity;

                out << YAML::EndMap; // DirectionalLightComponent
            }

            if (entity.HasComponent<ScriptComponent>())
            {
                out << YAML::Key << "ScriptComponent";
                out << YAML::BeginMap; // ScriptComponent
                auto& script = entity.GetComponent<ScriptComponent>();
                out << YAML::Key << "ModuleName" << YAML::Value << script.ModuleName;

                EntityInstanceData& data = ScriptEngine::GetEntityInstanceData(entity.GetSceneUUID(), uuid);
                const ScriptModuleFieldMap& moduleFieldMap = data.ModuleFieldMap;
                if (moduleFieldMap.find(script.ModuleName) != moduleFieldMap.end())
                {
                    const auto& fields = moduleFieldMap.at(script.ModuleName);
                    out << YAML::Key << "StoredFields" << YAML::Value;
                    out << YAML::BeginSeq;
                    for (const auto& [name, field] : fields)
                    {
                        out << YAML::BeginMap; // Field
                        out << YAML::Key << "Name" << YAML::Value << name;
                        out << YAML::Key << "Type" << YAML::Value << (Uint)field.mType;
                        out << YAML::Key << "Data" << YAML::Value;

                        switch (field.mType)
                        {
                            case FieldType::Int:
                                out << field.GetStoredValue<int>(); break;
                            case FieldType::UnsignedInt:
                                out << field.GetStoredValue<Uint>(); break;
                            case FieldType::Float:
                                out << field.GetStoredValue<float>(); break;
                            case FieldType::Vec2:
                                out << field.GetStoredValue<glm::vec2>(); break;
                            case FieldType::Vec3:
                                out << field.GetStoredValue<glm::vec3>(); break;
                            case FieldType::Vec4:
                                out << field.GetStoredValue<glm::vec4>(); break;
                            default: break;
                        }
                        out << YAML::EndMap; // Field
                    }
                    out << YAML::EndSeq;
                }

                out << YAML::EndMap; // ScriptComponent
            }

            if (entity.HasComponent<RigidBodyComponent>())
            {
                out << YAML::Key << "RigidBodyComponent";
                out << YAML::BeginMap; // RigidBodyComponent

                auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
                out << YAML::Key << "PhysicsMaterial" << YAML::Value << ""; // TODO
                out << YAML::Key << "BodyType" << YAML::Value << (int)rigidbodyComponent.BodyType;
                out << YAML::Key << "CollisionDetectionMode" << YAML::Value << (int)rigidbodyComponent.CollisionDetectionMode;
                out << YAML::Key << "Mass" << YAML::Value << rigidbodyComponent.Mass;
                out << YAML::Key << "LinearDrag" << YAML::Value << rigidbodyComponent.LinearDrag;
                out << YAML::Key << "AngularDrag" << YAML::Value << rigidbodyComponent.AngularDrag;
                out << YAML::Key << "DisableGravity" << YAML::Value << rigidbodyComponent.DisableGravity;
                out << YAML::Key << "IsKinematic" << YAML::Value << rigidbodyComponent.IsKinematic;

                out << YAML::Key << "Constraints";
                out << YAML::BeginMap; // Constraints

                out << YAML::Key << "LockPositionX" << YAML::Value << rigidbodyComponent.LockPositionX;
                out << YAML::Key << "LockPositionY" << YAML::Value << rigidbodyComponent.LockPositionY;
                out << YAML::Key << "LockPositionZ" << YAML::Value << rigidbodyComponent.LockPositionZ;
                out << YAML::Key << "LockRotationX" << YAML::Value << rigidbodyComponent.LockRotationX;
                out << YAML::Key << "LockRotationY" << YAML::Value << rigidbodyComponent.LockRotationY;
                out << YAML::Key << "LockRotationZ" << YAML::Value << rigidbodyComponent.LockRotationZ;

                out << YAML::EndMap;
                out << YAML::EndMap; // RigidBodyComponent
            }

            if (entity.HasComponent<BoxColliderComponent>())
            {
                out << YAML::Key << "BoxColliderComponent";
                out << YAML::BeginMap; // BoxColliderComponent

                BoxColliderComponent& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();
                out << YAML::Key << "Offset" << YAML::Value << boxColliderComponent.Offset;
                out << YAML::Key << "Size" << YAML::Value << boxColliderComponent.Size;
                out << YAML::Key << "IsTrigger" << YAML::Value << boxColliderComponent.IsTrigger;

                out << YAML::EndMap; // BoxColliderComponent
            }

            if (entity.HasComponent<SphereColliderComponent>())
            {
                out << YAML::Key << "SphereColliderComponent";
                out << YAML::BeginMap; // SphereColliderComponent

                SphereColliderComponent& sphereColliderComponent = entity.GetComponent<SphereColliderComponent>();
                out << YAML::Key << "Radius" << YAML::Value << sphereColliderComponent.Radius;
                out << YAML::Key << "IsTrigger" << YAML::Value << sphereColliderComponent.IsTrigger;

                out << YAML::EndMap; // SphereColliderComponent
            }

            if (entity.HasComponent<CapsuleColliderComponent>())
            {
                out << YAML::Key << "CapsuleColliderComponent";
                out << YAML::BeginMap; // CapsuleColliderComponent

                CapsuleColliderComponent& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();
                out << YAML::Key << "Radius" << YAML::Value << capsuleColliderComponent.Radius;
                out << YAML::Key << "Height" << YAML::Value << capsuleColliderComponent.Height;
                out << YAML::Key << "IsTrigger" << YAML::Value << capsuleColliderComponent.IsTrigger;

                out << YAML::EndMap; // CapsuleColliderComponent
            }

            if (entity.HasComponent<MeshColliderComponent>())
            {
                out << YAML::Key << "MeshColliderComponent";
                out << YAML::BeginMap; // MeshColliderComponent

                MeshColliderComponent& meshColliderComponent = entity.GetComponent<MeshColliderComponent>();

                if (meshColliderComponent.OverrideMesh)
                    out << YAML::Key << "AssetPath" << YAML::Value << meshColliderComponent.CollisionMesh->GetPath();

                out << YAML::Key << "IsConvex" << YAML::Value << meshColliderComponent.IsConvex;
                out << YAML::Key << "IsTrigger" << YAML::Value << meshColliderComponent.IsTrigger;
                out << YAML::Key << "OverrideMesh" << YAML::Value << meshColliderComponent.OverrideMesh;

                out << YAML::EndMap; // MeshColliderComponent
            }

            out << YAML::EndMap; // Entity
        }
    }

    void SceneSerializer::SerializeRendererSettings(YAML::Emitter& out)
    {
        const Scope<RendererData>& data = Renderer::GetData();
        Ref<EnvironmentMap>& environmentMapSlot = data->EnvironmentMap;

        out << YAML::Key << "Renderer Settings" << YAML::Value;
        out << YAML::BeginMap; // Renderer Settings

        // Environment Map
        out << YAML::Key << "EnvironmentMap AssetID"  << YAML::Value << (environmentMapSlot ? environmentMapSlot->GetHandle() : INVALID_ASSET_HANDLE);
        out << YAML::Key << "EnvironmentMap Bool"  << YAML::Value << data->EnvironmentMapActivated;
        out << YAML::Key << "TextureLOD" << YAML::Value << (environmentMapSlot ? environmentMapSlot->mTextureLOD : 0.0f);
        out << YAML::Key << "Intensity"  << YAML::Value << (environmentMapSlot ? environmentMapSlot->mIntensity : 1.0f);

        // Shadows
        out << YAML::Key << "ShadowMapResolution"  << YAML::Value << data->Shadows.GetShadowMapResolution();
        out << YAML::Key << "CascadeSplitLambda"  << YAML::Value << data->Shadows.GetCascadeSplitLambda();

        out << YAML::Key << "Exposure" << YAML::Value << data->Exposure;

        /////////////////////////////// PostProcessing ///////////////////////////////
        // - Bloom
        Bloom* bloom = data->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY);
        out << YAML::Key << "BloomEnabled" << YAML::Value << bloom->IsEnabled();
        out << YAML::Key << "BloomThreshold" << YAML::Value << bloom->GetBloomThreshold();
        out << YAML::Key << "GaussianSigma" << YAML::Value << bloom->GetGaussianSigma();

        // TODO: Remove
        out << YAML::Key << "Show Grid" << YAML::Value << data->ShowGrid;
        out << YAML::Key << "Show Camera Frustum" << YAML::Value << data->ShowCameraFrustum;
        out << YAML::Key << "Show BoundingBoxes" << YAML::Value << data->ShowAABB;

        out << YAML::EndMap; // Renderer Settings
    }

    void SceneSerializer::DeserializeRendererSettings(YAML::Node& data)
    {
        auto& settings = data["Renderer Settings"];
        const Scope<RendererData>& rendererData = Renderer::GetData();

        // Environment Map
        AssetHandle handle = settings["EnvironmentMap AssetID"].as<uint64_t>();
        rendererData->EnvironmentMap = AssetManager::GetAsset<EnvironmentMap>(handle);
        if (rendererData->EnvironmentMap)
        {
            rendererData->EnvironmentMapActivated = settings["EnvironmentMap Bool"].as<bool>();
            rendererData->EnvironmentMap->mTextureLOD = settings["TextureLOD"].as<float>();
            rendererData->EnvironmentMap->mIntensity = settings["Intensity"].as<float>();
        }

        rendererData->Exposure = settings["Exposure"].as<float>();

        /////////////////////////////// PostProcessing ///////////////////////////////
        // - Bloom
        Bloom* bloom = rendererData->PostProcessPipeline.GetEffectByKey<Bloom>(BLOOM_METHOD_KEY);
        bloom->SetEnabled(settings["BloomEnabled"].as<bool>());
        bloom->SetBloomThreshold(settings["BloomThreshold"].as<float>());
        bloom->SetGaussianSigma(settings["GaussianSigma"].as<float>());

        // Shadows
        rendererData->Shadows.Resize(settings["ShadowMapResolution"].as<Uint>());
        rendererData->Shadows.SetCascadeSplitLambda(settings["CascadeSplitLambda"].as<float>());

        // Debug Options
        rendererData->ShowGrid = settings["Show Grid"].as<bool>();
        rendererData->ShowCameraFrustum = settings["Show Camera Frustum"].as<bool>();
        rendererData->ShowAABB = settings["Show BoundingBoxes"].as<bool>();
    }

    void SceneSerializer::SerializePhysicsSettings(YAML::Emitter& out)
    {
        auto& settings = PhysicsEngine::GetSettings();
        out << YAML::Key << "Physics Settings" << YAML::Value;
        out << YAML::BeginMap; // Physics Settings
        out << YAML::Key << "FixedTimestep"            << YAML::Value << settings.FixedTimestep;
        out << YAML::Key << "Gravity"                  << YAML::Value << settings.Gravity;
        out << YAML::Key << "BroadphaseAlgorithm"      << YAML::Value << (int)settings.BroadphaseAlgorithm;
        out << YAML::Key << "WorldBoundsMin"           << YAML::Value << settings.WorldBoundsMin;
        out << YAML::Key << "WorldBoundsMax"           << YAML::Value << settings.WorldBoundsMax;
        out << YAML::Key << "WorldBoundsSubdivisions"  << YAML::Value << settings.WorldBoundsSubdivisions;
        out << YAML::Key << "FrictionModel"            << YAML::Value << (int)settings.FrictionModel;
        out << YAML::Key << "SolverIterations"         << YAML::Value << settings.SolverIterations;
        out << YAML::Key << "SolverVelocityIterations" << YAML::Value << settings.SolverVelocityIterations;
        out << YAML::EndMap; // Physics Settings
    }

    void SceneSerializer::DeserializePhysicsSettings(YAML::Node& data)
    {
        auto& settings = PhysicsEngine::GetSettings();
        auto savedPhysicsSettings = data["Physics Settings"];

        settings.FixedTimestep            = savedPhysicsSettings["FixedTimestep"].as<float>();
        settings.Gravity                  = savedPhysicsSettings["Gravity"].as<glm::vec3>();
        settings.BroadphaseAlgorithm      = static_cast<BroadphaseType>(savedPhysicsSettings["BroadphaseAlgorithm"].as<int>());
        settings.WorldBoundsMin           = savedPhysicsSettings["WorldBoundsMin"].as<glm::vec3>();
        settings.WorldBoundsMax           = savedPhysicsSettings["WorldBoundsMax"].as<glm::vec3>();
        settings.WorldBoundsSubdivisions  = savedPhysicsSettings["WorldBoundsSubdivisions"].as<Uint>();
        settings.FrictionModel            = static_cast<FrictionType>(savedPhysicsSettings["FrictionModel"].as<int>());
        settings.SolverIterations         = savedPhysicsSettings["SolverIterations"].as<Uint>();
        settings.SolverVelocityIterations = savedPhysicsSettings["SolverVelocityIterations"].as<Uint>();
    }

    void SceneSerializer::Serialize(const String& filepath)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Comment("Electro Scene File");
        out << YAML::Key << "Scene" << YAML::Value << mScene->GetUUID();

        SerializeRendererSettings(out);
        SerializePhysicsSettings(out);

        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        mScene->GetRegistry().each([&](auto entityID)
        {
            Entity entity = { entityID, mScene.Raw() };
            if (!entity) return;
            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        FileSystem::WriteFile(filepath, out.c_str());
    }

    bool SceneSerializer::Deserialize(const String& filepath)
    {
        Vector<String> missingPaths;
        YAML::Node data;
        try { data = YAML::LoadFile(filepath); }
        catch (const YAML::ParserException& ex)
        {
            Log::Error("Failed to load file '{0}'\n  {0}", filepath, ex.what());
        }

        if (!data["Scene"])
            return false;

        DeserializeRendererSettings(data);
        DeserializePhysicsSettings(data);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                String name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<String>();
                Entity deserializedEntity = mScene->CreateEntityWithID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    // Entities always have transforms
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                    auto& cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
                    cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                    cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
                    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                    cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

                    cc.Primary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto meshComponent = entity["MeshComponent"];
                if (meshComponent)
                {
                    const String meshPath = AssetManager::GetAbsolutePath(meshComponent["AssetPath"].as<String>());

                    if (!deserializedEntity.HasComponent<MeshComponent>())
                    {
                        MeshComponent& meshComp = deserializedEntity.AddComponent<MeshComponent>();
                        Ref<Mesh>& mesh = meshComp.Mesh;

                        if (!CheckPath(meshPath))
                            missingPaths.emplace_back(meshPath);
                        else
                            mesh = Mesh::Create(meshPath);
                        meshComp.CastShadows = meshComponent["CastShadows"].as<bool>();

                        // Deserialize Materials
                        size_t totalMaterials = meshComponent["Total Materials"].as<size_t>();
                        Vector<Ref<Material>>& materials = mesh->GetMaterials();
                        String key = "Material";

                        E_ASSERT(materials.size() == totalMaterials, "Material count doesn't match!");

                        for (size_t i = 0; i < totalMaterials; i++)
                        {
                            AssetHandle handle = meshComponent[fmt::format("{0}-{1}", key, i).c_str()].as<uint64_t>();
                            Ref<Material> emat = AssetManager::GetAsset<Material>(handle);
                            if (emat)
                            {
                                Ref<Material>& currentMaterial = materials[i];
                                if (currentMaterial)
                                    currentMaterial.Release();

                                materials[i] = emat;
                            }
                        }
                    }

                    Log::Info("Mesh Asset Path: {0}", meshPath);
                }

                auto pointLightComponent = entity["PointLightComponent"];
                if (pointLightComponent)
                {
                    if (!deserializedEntity.HasComponent<PointLightComponent>())
                    {
                        auto& component = deserializedEntity.AddComponent<PointLightComponent>();
                        component.Color = pointLightComponent["Color"].as<glm::vec3>();
                        component.Intensity = pointLightComponent["Intensity"].as<float>();
                        component.Radius = pointLightComponent["Radius"].as<float>();
                    }
                }

                auto dirLightComponent = entity["DirectionalLightComponent"];
                if (dirLightComponent)
                {
                    if (!deserializedEntity.HasComponent<DirectionalLightComponent>())
                    {
                        auto& component = deserializedEntity.AddComponent<DirectionalLightComponent>();
                        component.Color = dirLightComponent["Color"].as<glm::vec3>();
                        component.Intensity = dirLightComponent["Intensity"].as<float>();
                    }
                }

                auto scriptComponent = entity["ScriptComponent"];
                if (scriptComponent)
                {
                    if (!deserializedEntity.HasComponent<ScriptComponent>())
                    {
                        const String& moduleName = scriptComponent["ModuleName"].as<String>();
                        ScriptComponent& component = deserializedEntity.AddComponent<ScriptComponent>(moduleName);

                        if (ScriptEngine::ModuleExists(moduleName))
                        {
                            YAML::Node storedFields = scriptComponent["StoredFields"];
                            if (storedFields)
                            {
                                for (auto field : storedFields)
                                {
                                    String name = field["Name"].as<String>();
                                    String typeName = field["TypeName"] ? field["TypeName"].as<String>() : "";
                                    FieldType type = (FieldType)field["Type"].as<Uint>();

                                    EntityInstanceData& data = ScriptEngine::GetEntityInstanceData(mScene->GetUUID(), uuid);

                                    ScriptModuleFieldMap& moduleFieldMap = data.ModuleFieldMap;
                                    std::unordered_map<String, PublicField>& publicFields = moduleFieldMap[moduleName];

                                    //If field name is not found, emplace it to the map
                                    if (publicFields.find(name) == publicFields.end())
                                    {
                                        PublicField pf = { name, typeName, type };
                                        publicFields.emplace(name, std::move(pf));
                                    }

                                    auto dataNode = field["Data"];
                                    switch (type)
                                    {
                                        case FieldType::Float:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<float>()); break;
                                        }
                                        case FieldType::Int:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<int32_t>()); break;
                                        }
                                        case FieldType::UnsignedInt:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<Uint>()); break;
                                        }
                                        case FieldType::_String:
                                        {
                                            // TODO
                                            E_INTERNAL_ASSERT("Unimplemented"); break;
                                        }
                                        case FieldType::Vec2:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<glm::vec2>()); break;
                                        }
                                        case FieldType::Vec3:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<glm::vec3>()); break;
                                        }
                                        case FieldType::Vec4:
                                        {
                                            publicFields.at(name).SetStoredValue(dataNode.as<glm::vec4>()); break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                auto rigidBodyComponent = entity["RigidBodyComponent"];
                if (rigidBodyComponent)
                {
                    auto& component = deserializedEntity.AddComponent<RigidBodyComponent>();
                    String physicsMatPath = rigidBodyComponent["PhysicsMaterial"].as<String>();
                    component.PhysicsMaterial = Ref<PhysicsMaterial>::Create();

                    component.BodyType = (RigidBodyComponent::Type)rigidBodyComponent["BodyType"].as<int>();
                    component.CollisionDetectionMode = (RigidBodyComponent::CollisionDetectionType)rigidBodyComponent["CollisionDetectionMode"].as<int>();
                    component.Mass = rigidBodyComponent["Mass"].as<float>();
                    component.LinearDrag = rigidBodyComponent["LinearDrag"] ? rigidBodyComponent["LinearDrag"].as<float>() : 0.0f;
                    component.AngularDrag = rigidBodyComponent["AngularDrag"] ? rigidBodyComponent["AngularDrag"].as<float>() : 0.05f;
                    component.DisableGravity = rigidBodyComponent["DisableGravity"] ? rigidBodyComponent["DisableGravity"].as<bool>() : false;
                    component.IsKinematic = rigidBodyComponent["IsKinematic"] ? rigidBodyComponent["IsKinematic"].as<bool>() : false;

                    component.LockPositionX = rigidBodyComponent["Constraints"]["LockPositionX"].as<bool>();
                    component.LockPositionY = rigidBodyComponent["Constraints"]["LockPositionY"].as<bool>();
                    component.LockPositionZ = rigidBodyComponent["Constraints"]["LockPositionZ"].as<bool>();
                    component.LockRotationX = rigidBodyComponent["Constraints"]["LockRotationX"].as<bool>();
                    component.LockRotationY = rigidBodyComponent["Constraints"]["LockRotationY"].as<bool>();
                    component.LockRotationZ = rigidBodyComponent["Constraints"]["LockRotationZ"].as<bool>();
                }

                auto boxColliderComponent = entity["BoxColliderComponent"];
                if (boxColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<BoxColliderComponent>();
                    component.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
                    component.Size = boxColliderComponent["Size"].as<glm::vec3>();
                    component.IsTrigger = boxColliderComponent["IsTrigger"] ? boxColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateCube(component.Size);
                }

                auto sphereColliderComponent = entity["SphereColliderComponent"];
                if (sphereColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<SphereColliderComponent>();
                    component.Radius = sphereColliderComponent["Radius"].as<float>();
                    component.IsTrigger = sphereColliderComponent["IsTrigger"] ? sphereColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateSphere(component.Radius);
                }

                auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
                if (capsuleColliderComponent)
                {
                    auto& component = deserializedEntity.AddComponent<CapsuleColliderComponent>();
                    component.Radius = capsuleColliderComponent["Radius"].as<float>();
                    component.Height = capsuleColliderComponent["Height"].as<float>();
                    component.IsTrigger = capsuleColliderComponent["IsTrigger"] ? capsuleColliderComponent["IsTrigger"].as<bool>() : false;
                    component.DebugMesh = MeshFactory::CreateCapsule(component.Radius, component.Height);
                }

                auto meshColliderComponent = entity["MeshColliderComponent"];
                if (meshColliderComponent)
                {
                    Ref<Mesh> collisionMesh = deserializedEntity.HasComponent<MeshComponent>() ? deserializedEntity.GetComponent<MeshComponent>().Mesh : nullptr;
                    bool overrideMesh = meshColliderComponent["OverrideMesh"] ? meshColliderComponent["OverrideMesh"].as<bool>() : false;

                    if (overrideMesh)
                    {
                        String meshPath = meshColliderComponent["AssetPath"].as<String>();
                        if (!CheckPath(meshPath))
                            missingPaths.emplace_back(meshPath);
                        else
                            collisionMesh = Mesh::Create(meshPath);
                    }

                    if (collisionMesh)
                    {
                        auto& component = deserializedEntity.AddComponent<MeshColliderComponent>(collisionMesh);
                        component.IsConvex = meshColliderComponent["IsConvex"] ? meshColliderComponent["IsConvex"].as<bool>() : false;
                        component.IsTrigger = meshColliderComponent["IsTrigger"] ? meshColliderComponent["IsTrigger"].as<bool>() : false;
                        component.OverrideMesh = overrideMesh;

                        if (component.IsConvex)
                            PhysXInternal::CreateConvexMesh(component, deserializedEntity.Transform().Scale);
                        else
                            PhysXInternal::CreateTriangleMesh(component, deserializedEntity.Transform().Scale);
                    }
                    else
                        Log::Warn("MeshColliderComponent in use without valid mesh!");
                }
            }
        }

        if (!missingPaths.empty())
        {
            Log::Critical("The following files could not be loaded:");
            for (auto& path : missingPaths)
                Log::Critical("  {0}", path);
            return false;
        }

        return true;
    }
}
