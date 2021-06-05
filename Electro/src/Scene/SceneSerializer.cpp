//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "SceneSerializer.hpp"
#include "Physics/PhysicsEngine.hpp"
#include "Physics/PhysXInternal.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "EditorModule.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML
{
    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::quat>
    {
        static Node encode(const glm::quat& rhs)
        {
            Node node;
            node.push_back(rhs.w);
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::quat& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.w = node[0].as<float>();
            rhs.x = node[1].as<float>();
            rhs.y = node[2].as<float>();
            rhs.z = node[3].as<float>();
            return true;
        }
    };
}

namespace Electro
{
    static bool CheckPath(const String& path)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (f)
            fclose(f);
        return f != nullptr;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene, void* editorModule)
        : mScene(scene), mEditorModuleContext((EditorModule*)editorModule) {}

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

                auto mesh = entity.GetComponent<MeshComponent>().Mesh;
                out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();

                out << YAML::Key << "MaterialCount" << YAML::Value << mesh->GetMaterials().size();
                String key = "Material";
                for(Uint i = 0; i < mesh->GetMaterials().size(); i++)
                    out << YAML::Key << ("Material" + std::to_string(i)) << YAML::Value << mesh->GetMaterials()[i]->GetPath();

                out << YAML::EndMap; // MeshComponent
            }

            if (entity.HasComponent<PointLightComponent>())
            {
                out << YAML::Key << "PointLightComponent";
                out << YAML::BeginMap; // PointLightComponent

                auto& pointLight = entity.GetComponent<PointLightComponent>();
                out << YAML::Key << "Color" << YAML::Value << pointLight.Color;
                out << YAML::Key << "Intensity" << YAML::Value << pointLight.Intensity;

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
                out << YAML::EndMap; // ScriptComponent
            }

            if (entity.HasComponent<RigidBodyComponent>())
            {
                out << YAML::Key << "RigidBodyComponent";
                out << YAML::BeginMap; // RigidBodyComponent

                auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
                out << YAML::Key << "PhysicsMaterial" << YAML::Value << (rigidbodyComponent.PhysicsMaterial ? rigidbodyComponent.PhysicsMaterial->mPathInDisk : "");
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

                auto& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();
                out << YAML::Key << "Offset" << YAML::Value << boxColliderComponent.Offset;
                out << YAML::Key << "Size" << YAML::Value << boxColliderComponent.Size;
                out << YAML::Key << "IsTrigger" << YAML::Value << boxColliderComponent.IsTrigger;

                out << YAML::EndMap; // BoxColliderComponent
            }

            if (entity.HasComponent<SphereColliderComponent>())
            {
                out << YAML::Key << "SphereColliderComponent";
                out << YAML::BeginMap; // SphereColliderComponent

                auto& sphereColliderComponent = entity.GetComponent<SphereColliderComponent>();
                out << YAML::Key << "Radius" << YAML::Value << sphereColliderComponent.Radius;
                out << YAML::Key << "IsTrigger" << YAML::Value << sphereColliderComponent.IsTrigger;

                out << YAML::EndMap; // SphereColliderComponent
            }

            if (entity.HasComponent<CapsuleColliderComponent>())
            {
                out << YAML::Key << "CapsuleColliderComponent";
                out << YAML::BeginMap; // CapsuleColliderComponent

                auto& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();
                out << YAML::Key << "Radius" << YAML::Value << capsuleColliderComponent.Radius;
                out << YAML::Key << "Height" << YAML::Value << capsuleColliderComponent.Height;
                out << YAML::Key << "IsTrigger" << YAML::Value << capsuleColliderComponent.IsTrigger;

                out << YAML::EndMap; // CapsuleColliderComponent
            }

            if (entity.HasComponent<MeshColliderComponent>())
            {
                out << YAML::Key << "MeshColliderComponent";
                out << YAML::BeginMap; // MeshColliderComponent

                auto& meshColliderComponent = entity.GetComponent<MeshColliderComponent>();

                if (meshColliderComponent.OverrideMesh)
                    out << YAML::Key << "AssetPath" << YAML::Value << meshColliderComponent.CollisionMesh->GetFilePath();
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
        out << YAML::Key << "EnvironmentMap Path"  << YAML::Value << (environmentMapSlot ? environmentMapSlot->GetPath() : "");
        out << YAML::Key << "EnvironmentMap Bool"  << YAML::Value << data->EnvironmentMapActivated;
        out << YAML::Key << "TextureLOD" << YAML::Value << (environmentMapSlot ? environmentMapSlot->mTextureLOD : 0.0f);
        out << YAML::Key << "Intensity"  << YAML::Value << (environmentMapSlot ? environmentMapSlot->mIntensity : 1.0f);

        // Debug Options
        out << YAML::Key << "Show Grid" << YAML::Value << data->ShowGrid;
        out << YAML::Key << "Show Camera Frustum" << YAML::Value << data->ShowCameraFrustum;
        out << YAML::Key << "Show BoundingBoxes" << YAML::Value << data->ShowAABB;

        out << YAML::EndMap; // Renderer Settings
    }

    void SceneSerializer::DeserializeRendererSettings(YAML::Node& data)
    {
        auto& settings = data["Renderer Settings"];
        const Scope<RendererData>& rendererData = Renderer::GetData();

        if (CheckPath(settings["EnvironmentMap Path"].as<String>()))
        {
            rendererData->EnvironmentMap = Factory::CreateEnvironmentMap(settings["EnvironmentMap Path"].as<String>());
            rendererData->EnvironmentMapActivated = settings["EnvironmentMap Bool"].as<bool>();
            rendererData->EnvironmentMap->mTextureLOD = settings["TextureLOD"].as<float>();
            rendererData->EnvironmentMap->mIntensity = settings["Intensity"].as<float>();
        }

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
        out << YAML::Key << "FixedTimestep"                         << YAML::Value << settings.FixedTimestep;
        out << YAML::Key << "Gravity"                               << YAML::Value << settings.Gravity;
        out << YAML::Key << "BroadphaseAlgorithm"                   << YAML::Value << (int)settings.BroadphaseAlgorithm;
        out << YAML::Key << "WorldBoundsMin"                        << YAML::Value << settings.WorldBoundsMin;
        out << YAML::Key << "WorldBoundsMax"                        << YAML::Value << settings.WorldBoundsMax;
        out << YAML::Key << "WorldBoundsSubdivisions"               << YAML::Value << settings.WorldBoundsSubdivisions;
        out << YAML::Key << "FrictionModel"                         << YAML::Value << (int)settings.FrictionModel;
        out << YAML::Key << "SolverIterations"                      << YAML::Value << settings.SolverIterations;
        out << YAML::Key << "SolverVelocityIterations"              << YAML::Value << settings.SolverVelocityIterations;
        out << YAML::EndMap; // Physics Settings
    }

    void SceneSerializer::DeserializePhysicsSettings(YAML::Node& data)
    {
        auto& settings = PhysicsEngine::GetSettings();
        auto savedPhysicsSettings = data["Physics Settings"];

        settings.FixedTimestep                          = savedPhysicsSettings["FixedTimestep"].as<float>();
        settings.Gravity                                = savedPhysicsSettings["Gravity"].as<glm::vec3>();
        settings.BroadphaseAlgorithm                    = (BroadphaseType)savedPhysicsSettings["BroadphaseAlgorithm"].as<int>();
        settings.WorldBoundsMin                         = savedPhysicsSettings["WorldBoundsMin"].as<glm::vec3>();
        settings.WorldBoundsMax                         = savedPhysicsSettings["WorldBoundsMax"].as<glm::vec3>();
        settings.WorldBoundsSubdivisions                = savedPhysicsSettings["WorldBoundsSubdivisions"].as<Uint>();
        settings.FrictionModel                          = (FrictionType)savedPhysicsSettings["FrictionModel"].as<int>();
        settings.SolverIterations                       = savedPhysicsSettings["SolverIterations"].as<Uint>();
        settings.SolverVelocityIterations               = savedPhysicsSettings["SolverVelocityIterations"].as<Uint>();
    }

    void SceneSerializer::SerializeEditor(YAML::Emitter& out)
    {
        out << YAML::Key << "Editor Settings" << YAML::Value;
        out << YAML::BeginMap; // Editor Settings
        out << YAML::Key << "mVaultPath"                      << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mAssetsPath;
        out << YAML::Key << "mShowHierarchyAndInspectorPanel" << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowHierarchyAndInspectorPanel;
        out << YAML::Key << "mShowConsolePanel"               << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowConsolePanel;
        out << YAML::Key << "mShowVaultAndCachePanel"         << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowVaultAndCachePanel;
        out << YAML::Key << "mShowMaterialPanel"              << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowMaterialPanel;
        out << YAML::Key << "mShowRendererSettingsPanel"      << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowRendererSettingsPanel;
        out << YAML::Key << "mShowProfilerPanel"              << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowProfilerPanel;
        out << YAML::Key << "mShowPhysicsSettingsPanel"       << YAML::Value << ((EditorModule*)(mEditorModuleContext))->mShowPhysicsSettingsPanel;

        //Console
        auto console = Console::Get();
        out << YAML::Key << "mScrollLockEnabled"              << YAML::Value << console->mScrollLockEnabled;
        out << YAML::Key << "mTraceEnabled"                   << YAML::Value << console->mTraceEnabled;
        out << YAML::Key << "mInfoEnabled"                    << YAML::Value << console->mInfoEnabled;
        out << YAML::Key << "mDebugEnabled"                   << YAML::Value << console->mDebugEnabled;
        out << YAML::Key << "mWarningEnabled"                 << YAML::Value << console->mWarningEnabled;
        out << YAML::Key << "mErrorEnabled"                   << YAML::Value << console->mErrorEnabled;
        out << YAML::EndMap; // Editor Settings

    }

    void SceneSerializer::DeserializeEditor(YAML::Node& data)
    {
        auto savedSettings = data["Editor Settings"];
        ((EditorModule*)(mEditorModuleContext))->mAssetsPath                      = savedSettings["mVaultPath"].as<String>();
        ((EditorModule*)(mEditorModuleContext))->mShowHierarchyAndInspectorPanel = savedSettings["mShowHierarchyAndInspectorPanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowConsolePanel               = savedSettings["mShowConsolePanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowVaultAndCachePanel         = savedSettings["mShowVaultAndCachePanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowMaterialPanel              = savedSettings["mShowMaterialPanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowRendererSettingsPanel      = savedSettings["mShowRendererSettingsPanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowProfilerPanel              = savedSettings["mShowProfilerPanel"].as<bool>();
        ((EditorModule*)(mEditorModuleContext))->mShowPhysicsSettingsPanel       = savedSettings["mShowPhysicsSettingsPanel"].as<bool>();
        //Console
        auto console = Console::Get();
        console->mScrollLockEnabled                          = savedSettings["mScrollLockEnabled"].as<bool>();
        console->mTraceEnabled                               = savedSettings["mTraceEnabled"].as<bool>();
        console->mInfoEnabled                                = savedSettings["mInfoEnabled"].as<bool>();
        console->mDebugEnabled                               = savedSettings["mDebugEnabled"].as<bool>();
        console->mWarningEnabled                             = savedSettings["mWarningEnabled"].as<bool>();
        console->mErrorEnabled                               = savedSettings["mErrorEnabled"].as<bool>();
    }

    void SceneSerializer::Serialize(const String& filepath)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << mScene->GetUUID();
        SerializeRendererSettings(out);
        SerializePhysicsSettings(out);
        SerializeEditor(out);
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        mScene->mRegistry.each([&](auto entityID)
        {
            Entity entity = { entityID, mScene.Raw() };
            if (!entity) return;
            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        if (fout.bad())
        {
            ELECTRO_ERROR("Error serializing the file! Terminating serialization system...");
            return;
        }
        fout << out.c_str();
    }

    bool SceneSerializer::Deserialize(const String& filepath)
    {
        Vector<String> missingPaths;
        YAML::Node data;
        try { data = YAML::LoadFile(filepath); }
        catch (const YAML::ParserException& ex)
        {
            ELECTRO_ERROR("Failed to load .electro file '%s'\n  %s", filepath.c_str(), ex.what());
        }

        if (!data["Scene"])
            return false;

        DeserializeRendererSettings(data);
        DeserializePhysicsSettings(data);
        DeserializeEditor(data);
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
                    const String meshPath = meshComponent["AssetPath"].as<String>();

                    if (!deserializedEntity.HasComponent<MeshComponent>())
                    {
                        Ref<Mesh>& mesh = deserializedEntity.AddComponent<MeshComponent>().Mesh;
                        if (!CheckPath(meshPath))
                            missingPaths.emplace_back(meshPath);
                        else
                            mesh = Factory::CreateMesh(meshPath);

                        Uint count = meshComponent["MaterialCount"].as<Uint>();
                        String key = "Material";
                        Vector<Ref<Material>>& materials = mesh->GetMaterials();
                        materials.resize(count);
                        for(Uint i = 0; i < count; i++)
                        {
                            const String& path = meshComponent[key + std::to_string(i)].as<String>();
                            materials[i] = Factory::CreateMaterial(AssetManager::Get<Shader>("PBR.hlsl"), "Material", path);
                            materials[i]->Deserialize();
                        }
                    }

                    ELECTRO_INFO("Mesh Asset Path: %s", meshPath.c_str());
                }

                auto pointLightComponent = entity["PointLightComponent"];
                if (pointLightComponent)
                {
                    if (!deserializedEntity.HasComponent<PointLightComponent>())
                    {
                        auto& component = deserializedEntity.AddComponent<PointLightComponent>();
                        component.Color = pointLightComponent["Color"].as<glm::vec3>();
                        component.Intensity = pointLightComponent["Intensity"].as<float>();
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
                        auto& component = deserializedEntity.AddComponent<ScriptComponent>();
                        component.ModuleName = scriptComponent["ModuleName"].as<String>();
                    }
                }

                auto rigidBodyComponent = entity["RigidBodyComponent"];
                if (rigidBodyComponent)
                {
                    auto& component = deserializedEntity.AddComponent<RigidBodyComponent>();
                    String physicsMatPath = rigidBodyComponent["PhysicsMaterial"].as<String>();
                    component.PhysicsMaterial = (physicsMatPath.empty() ? PhysicsEngine::GetGlobalPhysicsMaterial() : Factory::CreatePhysicsMaterial(physicsMatPath));

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
                }

                auto meshColliderComponent = entity["MeshColliderComponent"];
                if (meshColliderComponent)
                {
                    Ref<Mesh> collisionMesh = deserializedEntity.HasComponent<MeshComponent>() ? deserializedEntity.GetComponent<MeshComponent>().Mesh : nullptr;
                    bool overrideMesh = meshColliderComponent["OverrideMesh"] ? meshColliderComponent["OverrideMesh"].as<bool>() : false;

                    if (overrideMesh)
                    {
                        std::string meshPath = meshColliderComponent["AssetPath"].as<std::string>();
                        if (!CheckPath(meshPath))
                            missingPaths.emplace_back(meshPath);
                        else
                            collisionMesh = Factory::CreateMesh(meshPath);
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
                        ELECTRO_WARN("MeshColliderComponent in use without valid mesh!");
                }
            }
        }

        if (missingPaths.size())
        {
            ELECTRO_ERROR("The following files could not be loaded:");
            for (auto& path : missingPaths)
                ELECTRO_ERROR("  %s", path.c_str());
            return false;
        }

        return true;
    }
}
