//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroUUID.hpp"
#include "Core/ElectroVault.hpp"
#include "EDevice/EDevice.hpp"
#include "Renderer/Interface/ElectroTexture.hpp"
#include "Renderer/ElectroEnvironmentMap.hpp"
#include "Renderer/ElectroMesh.hpp"
#include "Renderer/ElectroMeshFactory.hpp"
#include "ElectroSceneCamera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <FontAwesome.hpp>

namespace Electro
{
    struct IDComponent
    {
        UUID ID = 0;
    };

    struct TagComponent
    {
        String Tag;
        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const String tag)
            :Tag(tag) {}
    };

    struct TransformComponent
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
            :Translation(translation), Rotation(rotation), Scale(scale) {}
        TransformComponent(const glm::vec3& translation)
            :Translation(translation) {}

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
            return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }
        void Reset() { Translation = { 0.0f, 0.0f, 0.0f }; Rotation = { 0.0f, 0.0f, 0.0f }; Scale = { 1.0f, 1.0f, 1.0f }; }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> Texture = nullptr;
        String TextureFilepath;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            :Color(color) {}

        void SetTexture(const String& filepath)
        {
            Texture = EDevice::CreateTexture2D(filepath);
            Vault::Submit<Texture2D>(Texture);
            TextureFilepath = filepath;
        }

        void RemoveTexture() { Texture = nullptr; TextureFilepath = ""; }

        void Reset()
        {
            Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;

        void Reset()
        {
            Primary = true;
            FixedAspectRatio = false;
        }
    };

    struct MeshComponent
    {
        Ref<Electro::Mesh> Mesh;
        String MeshFilepath;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;

        void SetFilePath(String& path) { MeshFilepath = path; }
        void Reset() { Mesh = nullptr; MeshFilepath.clear(); }
    };

    struct SkyLightComponent
    {
        String EnvironmentMapPath = "";
        Ref<Electro::EnvironmentMap> EnvironmentMap = nullptr;
        SkyLightComponent() = default;
        SkyLightComponent(Ref<Electro::EnvironmentMap> envMap)
            : EnvironmentMap(envMap) {}

        void Reset() { EnvironmentMap = nullptr; }
    };

    struct PointLightComponent
    {
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;

        PointLightComponent() = default;
        PointLightComponent(glm::vec3 color, float intensity)
            : Color(color), Intensity(intensity) {}

        void Reset() { Color = { 1.0f, 1.0f, 1.0f }; Intensity = 1.0f; }
    };

    struct ScriptComponent
    {
        String ModuleName;
        ScriptComponent() = default;
        ScriptComponent(String& moduleName)
            :ModuleName(moduleName) {}

        void Reset() { ModuleName = "ElectroNull"; }
    };

    //PhysX physics
    struct RigidBodyComponent
    {
        enum class Type { Static, Dynamic };
        enum class CollisionDetectionType { Discrete = 0, Continious = 1 };
        Type BodyType;
        CollisionDetectionType CollisionDetectionMode = CollisionDetectionType::Discrete;
        float Mass = 1.0f;
        float LinearDrag = 0.0f;
        float AngularDrag = 0.05f;
        bool DisableGravity = false;
        bool IsKinematic = false;

        bool LockPositionX = false;
        bool LockPositionY = false;
        bool LockPositionZ = false;
        bool LockRotationX = false;
        bool LockRotationY = false;
        bool LockRotationZ = false;

        RigidBodyComponent() = default;
        RigidBodyComponent(const RigidBodyComponent& other) = default;
        void Reset()
        {
            Mass = 1.0f;
            LinearDrag = 0.0f;
            AngularDrag = 0.05f;
            CollisionDetectionMode = CollisionDetectionType::Discrete;
            DisableGravity = false;
            IsKinematic = false;
            LockPositionX = false;
            LockPositionY = false;
            LockPositionZ = false;
            LockRotationX = false;
            LockRotationY = false;
            LockRotationZ = false;
        }
    };

    struct PhysicsMaterialComponent //TODO: Make it a resource
    {
        float StaticFriction = 1.0f;
        float DynamicFriction = 1.0f;
        float Bounciness = 1.0f;

        PhysicsMaterialComponent() = default;
        PhysicsMaterialComponent(const PhysicsMaterialComponent& other) = default;
        void Reset()
        {
            StaticFriction = 1.0f;
            DynamicFriction = 1.0f;
            Bounciness = 1.0f;
        }
    };

    struct BoxColliderComponent
    {
        glm::vec3 Size = { 1.0f, 1.0f, 1.0f };
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
        bool IsTrigger = false;

        Ref<Mesh> DebugMesh = MeshFactory::CreateCube(Size); //DebugMesh, for showing collider bounds
        BoxColliderComponent() = default;
        BoxColliderComponent(const BoxColliderComponent& other) = default;

        void Reset()
        {
            Size = { 1.0f, 1.0f, 1.0f };
            Offset = { 0.0f, 0.0f, 0.0f };
            IsTrigger = false;
        }
    };

    struct SphereColliderComponent
    {
        float Radius = 0.5f;
        bool IsTrigger = false;

        Ref<Mesh> DebugMesh = MeshFactory::CreateSphere(Radius); //DebugMesh, for showing collider bounds

        SphereColliderComponent() = default;
        SphereColliderComponent(const SphereColliderComponent& other) = default;
        void Reset()
        {
            Radius = 0.5f;
            IsTrigger = false;
        }
    };

    struct CapsuleColliderComponent
    {
        float Radius = 0.5f;
        float Height = 1.0f;
        bool IsTrigger = false;

        CapsuleColliderComponent() = default;
        CapsuleColliderComponent(const CapsuleColliderComponent& other) = default;

        void Reset()
        {
            Radius = 0.5f;
            Height = 1.0f;
            IsTrigger = false;
        }
    };

    struct MeshColliderComponent
    {
        Ref<Mesh> CollisionMesh;
        Vector<Ref<Mesh>> ProcessedMeshes; //Storage for debug mesh

        bool IsConvex = false;
        bool IsTrigger = false;
        bool OverrideMesh = false;

        MeshColliderComponent() = default;
        MeshColliderComponent(const MeshColliderComponent& other) = default;
        MeshColliderComponent(const Ref<Mesh>& mesh)
            : CollisionMesh(mesh) {}

        void Reset()
        {
            IsConvex = false;
            IsTrigger = false;
            OverrideMesh = false;
        }
        operator Ref<Mesh>() { return CollisionMesh; }
    };
}
