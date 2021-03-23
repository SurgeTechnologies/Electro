//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroUUID.hpp"
#include "Core/ElectroVault.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroMesh.hpp"
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
            Texture = Texture2D::Create(filepath);
            Vault::Submit<Texture2D>(Texture);
            TextureFilepath = filepath;
        }

        void RemoveTexture() { Texture = nullptr; TextureFilepath = ""; }

        void Reset()
        {
            Color = { 1.0f, 1.0f, 1.0f, 1.0f };
            RemoveTexture();
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
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 0.2f;

        SkyLightComponent() = default;
        SkyLightComponent(glm::vec3 color, float intensity) :
            Color(color), Intensity(intensity) {}
        void Reset() { Color = { 1.0f, 1.0f, 1.0f }; Intensity = 0.2f; }
    };

    struct PointLightComponent
    {
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;

        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;

        PointLightComponent() = default;
        PointLightComponent(float constant, float linear, float quadratic, glm::vec3 color, float intensity) :
            Constant(constant), Linear(linear), Quadratic(quadratic), Color(color), Intensity(intensity) {}

        void Reset()
        {
            Color = { 1.0f, 1.0f, 1.0f };
            Intensity = 1.0f;
            Constant = 1.0f;
            Linear = 0.09f;
            Quadratic = 0.032f;
        }
    };

}
