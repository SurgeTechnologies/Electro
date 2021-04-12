//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroInput.hpp"
#include "Physics/ElectroPhysicsEngine.hpp"
#include "Scene/ElectroComponents.hpp"
#include <mono/metadata/object.h>
#include <mono/metadata/object-forward.h>

namespace Electro::Scripting
{
    // Console
    void Electro_Console_LogInfo(MonoObject* message);
    void Electro_Console_LogWarn(MonoObject* message);
    void Electro_Console_LogDebug(MonoObject* message);
    void Electro_Console_LogError(MonoObject* message);
    void Electro_Console_LogCritical(MonoObject* message);

    //Input
    bool Electro_Input_IsKeyPressed(KeyCode key);
    bool Electro_Input_IsMouseButtonPressed(MouseCode button);
    void Electro_Input_GetMousePosition(glm::vec2* outPosition);

    //Physics
    bool Electro_Physics_Raycast(RaycastHit* hit, glm::vec3* origin, glm::vec3* direction, float maxDistance);

    //Entity
    void Electro_Entity_CreateComponent(uint64_t entityID, void* type);
    bool Electro_Entity_HasComponent(uint64_t entityID, void* type);
    uint64_t Electro_Entity_FindEntityByTag(MonoString* tag);
    bool Electro_Entity_EntityExists(uint64_t entityID);

    //Tag Component
    MonoString* Electro_TagComponent_GetTag(uint64_t entityID);
    void Electro_TagComponent_SetTag(uint64_t entityID, MonoString* inTag);

    //Transform Component
    void Electro_TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform);
    void Electro_TransformComponent_SetTransform(uint64_t entityID, TransformComponent* inTransform);
    void Electro_TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation);
    void Electro_TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation);
    void Electro_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation);
    void Electro_TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation);
    void Electro_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale);
    void Electro_TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale);

    //SpriteRenderer Component
    void Electro_SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
    void Electro_SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor);

    //Camera Component
    void Electro_CameraComponent_SetAsPrimary(uint64_t entityID, bool* isPrimary);
    bool Electro_CameraComponent_IsPrimary(uint64_t entityID);
    void Electro_CameraComponent_SetFixedAspectRatio(uint64_t entityID, bool isAspectRatioFixed);
    bool Electro_CameraComponent_IsFixedAspectRatio(uint64_t entityID);

    //Rigidbody Component
    RigidBodyComponent::Type Electro_RigidBodyComponent_GetBodyType(uint64_t entityID);
    void Electro_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode forceMode);
    void Electro_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode);
    void Electro_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity);
    void Electro_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity);
    void Electro_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity);
    void Electro_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity);
    void Electro_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation);
    float Electro_RigidBodyComponent_GetMass(uint64_t entityID);
    void Electro_RigidBodyComponent_SetMass(uint64_t entityID, float mass);
    void Electro_RigidBodyComponent_UseGravity(uint64_t entityID, bool use);
}