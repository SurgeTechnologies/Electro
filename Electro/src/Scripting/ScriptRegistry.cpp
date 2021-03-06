//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ScriptRegistry.hpp"
#include "ScriptWrappers.hpp"
#include <mono/metadata/reflection.h>

namespace Electro
{
    std::unordered_map<MonoType*, std::function<bool(Entity&)>> sHasComponentFuncs;
    std::unordered_map<MonoType*, std::function<void(Entity&)>> sCreateComponentFuncs;

    extern MonoImage* sCoreAssemblyImage;

#define RegisterComponent(Type)                                                                    \
    {                                                                                              \
        MonoType* type = mono_reflection_type_from_name("Electro." #Type, sCoreAssemblyImage);     \
        if (type)                                                                                  \
        {                                                                                          \
            Uint id = mono_type_get_type(type);                                                    \
            sHasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); }; \
            sCreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };     \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            Log::Error("No C# component class found for " #Type "!");                              \
        }                                                                                          \
    }

    void ScriptRegistry::RegisterAll()
    {
        sHasComponentFuncs.clear();
        sCreateComponentFuncs.clear();
        RegisterComponent(TagComponent);
        RegisterComponent(TransformComponent);
        RegisterComponent(CameraComponent);
        RegisterComponent(RigidBodyComponent);

        //Logging
        mono_add_internal_call("Electro.Console::LogInfo_Native",  Scripting::Electro_Console_LogInfo);
        mono_add_internal_call("Electro.Console::LogWarn_Native",  Scripting::Electro_Console_LogWarn);
        mono_add_internal_call("Electro.Console::LogDebug_Native", Scripting::Electro_Console_LogDebug);
        mono_add_internal_call("Electro.Console::LogError_Native", Scripting::Electro_Console_LogError);
        mono_add_internal_call("Electro.Console::LogFatal_Native", Scripting::Electro_Console_LogCritical);

        //Input
        mono_add_internal_call("Electro.Input::IsKeyPressed_Native",         Scripting::Electro_Input_IsKeyPressed);
        mono_add_internal_call("Electro.Input::IsMouseButtonPressed_Native", Scripting::Electro_Input_IsMouseButtonPressed);
        mono_add_internal_call("Electro.Input::GetMousePosition_Native",     Scripting::Electro_Input_GetMousePosition);

        //Physics
        mono_add_internal_call("Electro.Physics::Raycast_Native", Scripting::Electro_Physics_Raycast);
        mono_add_internal_call("Electro.Physics::SetFixedTimestep_Native", Scripting::Electro_Physics_SetFixedTimestep);
        mono_add_internal_call("Electro.Physics::GetFixedTimestep_Native", Scripting::Electro_Physics_GetFixedTimestep);
        mono_add_internal_call("Electro.Physics::SetGravity_Native", Scripting::Electro_Physics_SetGravity);
        mono_add_internal_call("Electro.Physics::GetGravity_Native", Scripting::Electro_Physics_GetGravity);
        mono_add_internal_call("Electro.Physics::SetSolverIterations_Native", Scripting::Electro_Physics_SetSolverIterations);
        mono_add_internal_call("Electro.Physics::GetSolverIterations_Native", Scripting::Electro_Physics_GetSolverIterations);
        mono_add_internal_call("Electro.Physics::SetSolverVelocityIterations_Native", Scripting::Electro_Physics_SetSolverVelocityIterations);
        mono_add_internal_call("Electro.Physics::GetSolverVelocityIterations_Native", Scripting::Electro_Physics_GetSolverVelocityIterations);

        //Entity
        mono_add_internal_call("Electro.Entity::CreateComponent_Native", Scripting::Electro_Entity_CreateComponent);
        mono_add_internal_call("Electro.Entity::HasComponent_Native",    Scripting::Electro_Entity_HasComponent);
        mono_add_internal_call("Electro.Entity::FindEntityByTag_Native", Scripting::Electro_Entity_FindEntityByTag);
        mono_add_internal_call("Electro.Entity::EntityExists_Native", Scripting::Electro_Entity_EntityExists);

        //Tag
        mono_add_internal_call("Electro.TagComponent::GetTag_Native", Scripting::Electro_TagComponent_GetTag);
        mono_add_internal_call("Electro.TagComponent::SetTag_Native", Scripting::Electro_TagComponent_SetTag);

        //Transform
        mono_add_internal_call("Electro.TransformComponent::GetTransform_Native",   Scripting::Electro_TransformComponent_GetTransform);
        mono_add_internal_call("Electro.TransformComponent::SetTransform_Native",   Scripting::Electro_TransformComponent_SetTransform);
        mono_add_internal_call("Electro.TransformComponent::GetTranslation_Native", Scripting::Electro_TransformComponent_GetTranslation);
        mono_add_internal_call("Electro.TransformComponent::SetTranslation_Native", Scripting::Electro_TransformComponent_SetTranslation);
        mono_add_internal_call("Electro.TransformComponent::GetRotation_Native",    Scripting::Electro_TransformComponent_GetRotation);
        mono_add_internal_call("Electro.TransformComponent::SetRotation_Native",    Scripting::Electro_TransformComponent_SetRotation);
        mono_add_internal_call("Electro.TransformComponent::GetScale_Native",       Scripting::Electro_TransformComponent_GetScale);
        mono_add_internal_call("Electro.TransformComponent::SetScale_Native",       Scripting::Electro_TransformComponent_SetScale);

        //Camera
        mono_add_internal_call("Electro.CameraComponent::IsPrimary_Native",           Scripting::Electro_CameraComponent_IsPrimary);
        mono_add_internal_call("Electro.CameraComponent::SetAsPrimary_Native",        Scripting::Electro_CameraComponent_SetAsPrimary);
        mono_add_internal_call("Electro.CameraComponent::IsFixedAspectRatio_Native",  Scripting::Electro_CameraComponent_IsFixedAspectRatio);
        mono_add_internal_call("Electro.CameraComponent::SetFixedAspectRatio_Native", Scripting::Electro_CameraComponent_SetFixedAspectRatio);

        //Rigidbody
        mono_add_internal_call("Electro.RigidBodyComponent::GetBodyType_Native",        Scripting::Electro_RigidBodyComponent_GetBodyType);
        mono_add_internal_call("Electro.RigidBodyComponent::AddForce_Native",           Scripting::Electro_RigidBodyComponent_AddForce);
        mono_add_internal_call("Electro.RigidBodyComponent::AddTorque_Native",          Scripting::Electro_RigidBodyComponent_AddTorque);
        mono_add_internal_call("Electro.RigidBodyComponent::GetLinearVelocity_Native",  Scripting::Electro_RigidBodyComponent_GetLinearVelocity);
        mono_add_internal_call("Electro.RigidBodyComponent::SetLinearVelocity_Native",  Scripting::Electro_RigidBodyComponent_SetLinearVelocity);
        mono_add_internal_call("Electro.RigidBodyComponent::GetAngularVelocity_Native", Scripting::Electro_RigidBodyComponent_GetAngularVelocity);
        mono_add_internal_call("Electro.RigidBodyComponent::SetAngularVelocity_Native", Scripting::Electro_RigidBodyComponent_SetAngularVelocity);
        mono_add_internal_call("Electro.RigidBodyComponent::Rotate_Native",             Scripting::Electro_RigidBodyComponent_Rotate);
        mono_add_internal_call("Electro.RigidBodyComponent::GetMass_Native",            Scripting::Electro_RigidBodyComponent_GetMass);
        mono_add_internal_call("Electro.RigidBodyComponent::SetMass_Native",            Scripting::Electro_RigidBodyComponent_SetMass);
        mono_add_internal_call("Electro.RigidBodyComponent::UseGravity_Native",         Scripting::Electro_RigidBodyComponent_UseGravity);
    }
}
