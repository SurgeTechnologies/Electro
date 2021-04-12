//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptWrappers.hpp"
#include "ElectroMonoUtils.hpp"
#include "Physics/ElectroPhysicsActor.hpp"
#include "mono/metadata/appdomain.h"
#include <mono/metadata/reflection.h>
#include <mono/metadata/object-forward.h>

namespace Electro
{
    extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> sHasComponentFuncs;
    extern std::unordered_map<MonoType*, std::function<void(Entity&)>> sCreateComponentFuncs;
}

namespace Electro::Scripting
{
    void Electro_Console_LogInfo(MonoObject* message)
    {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_INFO(msg);
    }
    void Electro_Console_LogWarn(MonoObject* message)
    {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_WARN(msg);
    }
    void Electro_Console_LogDebug(MonoObject* message)
    {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_DEBUG(msg);
    }
    void Electro_Console_LogError(MonoObject* message)
    {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_ERROR(msg);
    }
    void Electro_Console_LogCritical(MonoObject* message)
    {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_CRITICAL(msg);
    }

    bool Electro_Input_IsKeyPressed(KeyCode key)
    {
        return Input::IsKeyPressed(key);
    }

    bool Electro_Input_IsMouseButtonPressed(MouseCode button)
    {
        return Input::IsMouseButtonPressed(button);
    }

    void Electro_Input_GetMousePosition(glm::vec2* outPosition)
    {
        glm::vec2 result = Input::GetMousePosition();
        *outPosition = result;
    }

    bool Electro_Physics_Raycast(RaycastHit* hit, glm::vec3* origin, glm::vec3* direction, float maxDistance)
    {
        return PhysicsEngine::Raycast(hit, *origin, *direction, maxDistance);
    }

    void Electro_Entity_CreateComponent(uint64_t entityID, void* type)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);

        MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
        sCreateComponentFuncs[monoType](entity);
    }

    bool Electro_Entity_HasComponent(uint64_t entityID, void* type)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);

        MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
        bool result = sHasComponentFuncs[monoType](entity);
        return result;
    }

    uint64_t Electro_Entity_FindEntityByTag(MonoString* tag)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");

        Entity entity = scene->FindEntityByTag(ConvertMonoStringToCppString(tag));
        if (entity)
            return entity.GetComponent<IDComponent>().ID;

        return 0;
    }

    bool Electro_Entity_EntityExists(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        return scene->EntityExists(entityID);
    }

    MonoString* Electro_TagComponent_GetTag(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TagComponent>();
        String tag = component.Tag;
        return ConvertCppStringToMonoString(mono_domain_get(), tag.c_str());
    }

    void Electro_TagComponent_SetTag(uint64_t entityID, MonoString* inTag)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TagComponent>();
        String& tag = ConvertMonoStringToCppString(inTag);
        component.Tag = tag;
    }

    void Electro_TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        *outTransform = component;
    }

    void Electro_TransformComponent_SetTransform(uint64_t entityID, TransformComponent* inTransform)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        component = *inTransform;
    }

    void Electro_TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        *outTranslation = component.Translation;
    }

    void Electro_TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        component.Translation = *inTranslation;
    }

    void Electro_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        *outRotation = component.Rotation;
    }

    void Electro_TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        component.Rotation = *inRotation;
    }

    void Electro_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        *outScale = component.Scale;
    }

    void Electro_TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<TransformComponent>();
        component.Scale = *inScale;
    }

    void Electro_SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<SpriteRendererComponent>();
        *outColor = component.Color;
    }

    void Electro_SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<SpriteRendererComponent>();
        component.Color = *inColor;
    }

    void Electro_CameraComponent_SetAsPrimary(uint64_t entityID, bool* isPrimary)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<CameraComponent>();
        component.Primary = isPrimary;
    }

    bool Electro_CameraComponent_IsPrimary(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<CameraComponent>();
        return component.Primary;
    }

    void Electro_CameraComponent_SetFixedAspectRatio(uint64_t entityID, bool isAspectRatioFixed)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<CameraComponent>();
        component.FixedAspectRatio = isAspectRatioFixed;
    }

    bool Electro_CameraComponent_IsFixedAspectRatio(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<CameraComponent>();
        return component.FixedAspectRatio;
    }

    RigidBodyComponent::Type Electro_RigidBodyComponent_GetBodyType(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<RigidBodyComponent>();
        return component.BodyType;
    }

    void Electro_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode forceMode)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<RigidBodyComponent>();
        if (component.IsKinematic)
        {
            ELECTRO_WARN("Cannot add force to a Kinematic Actor! Please uncheck the Is Kinematic or change the script C# code. EntityID: (%llx)", entityID);
            return;
        }
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->AddForce(*force, forceMode);
    }

    void Electro_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<RigidBodyComponent>();
        if (component.IsKinematic)
        {
            ELECTRO_WARN("Cannot add torque to a Kinematic Actor! Please uncheck the Is Kinematic or change the script C# code. EntityID: (%llx)", entityID);
            return;
        }
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->AddTorque(*torque, forceMode);
    }

    void Electro_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        *outVelocity = actor->GetLinearVelocity();
    }

    void Electro_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->SetLinearVelocity(*velocity);
    }

    void Electro_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        *outVelocity = actor->GetAngularVelocity();
    }

    void Electro_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->SetAngularVelocity(*velocity);
    }

    void Electro_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->Rotate(*rotation);
    }

    float Electro_RigidBodyComponent_GetMass(uint64_t entityID)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);

        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        return actor->GetMass();
    }

    void Electro_RigidBodyComponent_SetMass(uint64_t entityID, float mass)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);

        Ref<PhysicsActor> actor = PhysicsEngine::GetActorForEntity(entity);
        actor->SetMass(mass);
    }

    void Electro_RigidBodyComponent_UseGravity(uint64_t entityID, bool use)
    {
        Ref<Scene> scene = ScriptEngine::GetSceneContext();
        E_ASSERT(scene, "No active scene!");
        const auto& entityMap = scene->GetEntityMap();
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
        Entity entity = entityMap.at(entityID);
        auto& component = entity.GetComponent<RigidBodyComponent>();
        component.DisableGravity = use;
    }
}
