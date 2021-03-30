//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptRegistry.hpp"
#include "ElectroMonoUtils.hpp"
#include "ElectroScriptWrappers.hpp"
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
            sHasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
            sCreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };    \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            ELECTRO_ERROR("No C# component class found for " #Type "!");                           \
        }                                                                                          \
    }

    void ScriptRegistry::RegisterAll()
    {
        RegisterComponent(TagComponent);
        RegisterComponent(TransformComponent);
        RegisterComponent(CameraComponent);
        RegisterComponent(SpriteRendererComponent);

        mono_add_internal_call("Electro.Console::LogInfo_Native", Electro::Scripting::Electro_Console_LogInfo);
        mono_add_internal_call("Electro.Console::LogWarn_Native", Electro::Scripting::Electro_Console_LogWarn);
        mono_add_internal_call("Electro.Console::LogDebug_Native", Electro::Scripting::Electro_Console_LogDebug);
        mono_add_internal_call("Electro.Console::LogError_Native", Electro::Scripting::Electro_Console_LogError);
        mono_add_internal_call("Electro.Console::LogFatal_Native", Electro::Scripting::Electro_Console_LogCritical);

        mono_add_internal_call("Electro.Input::IsKeyPressed_Native", Electro::Scripting::Electro_Input_IsKeyPressed);
        mono_add_internal_call("Electro.Input::IsMouseButtonPressed_Native", Electro::Scripting::Electro_Input_IsMouseButtonPressed);
        mono_add_internal_call("Electro.Input::GetMousePosition_Native", Electro::Scripting::Electro_Input_GetMousePosition);

        mono_add_internal_call("Electro.Entity::CreateComponent_Native", Electro::Scripting::Electro_Entity_CreateComponent);
        mono_add_internal_call("Electro.Entity::HasComponent_Native", Electro::Scripting::Electro_Entity_HasComponent);
        mono_add_internal_call("Electro.Entity::FindEntityByTag_Native", Electro::Scripting::Electro_Entity_FindEntityByTag);

        mono_add_internal_call("Electro.TagComponent::GetTag_Native", Electro::Scripting::Electro_TagComponent_GetTag);
        mono_add_internal_call("Electro.TagComponent::SetTag_Native", Electro::Scripting::Electro_TagComponent_SetTag);

        mono_add_internal_call("Electro.TransformComponent::GetTransform_Native", Electro::Scripting::Electro_TransformComponent_GetTransform);
        mono_add_internal_call("Electro.TransformComponent::SetTransform_Native", Electro::Scripting::Electro_TransformComponent_SetTransform);
        mono_add_internal_call("Electro.TransformComponent::GetTranslation_Native", Electro::Scripting::Electro_TransformComponent_GetTranslation);
        mono_add_internal_call("Electro.TransformComponent::SetTranslation_Native", Electro::Scripting::Electro_TransformComponent_SetTranslation);
        mono_add_internal_call("Electro.TransformComponent::GetRotation_Native", Electro::Scripting::Electro_TransformComponent_GetRotation);
        mono_add_internal_call("Electro.TransformComponent::SetRotation_Native", Electro::Scripting::Electro_TransformComponent_SetRotation);
        mono_add_internal_call("Electro.TransformComponent::GetScale_Native", Electro::Scripting::Electro_TransformComponent_GetScale);
        mono_add_internal_call("Electro.TransformComponent::SetScale_Native", Electro::Scripting::Electro_TransformComponent_SetScale);

        mono_add_internal_call("Electro.SpriteRendererComponent::GetColor_Native", Electro::Scripting::Electro_SpriteRendererComponent_GetColor);
        mono_add_internal_call("Electro.SpriteRendererComponent::SetColor_Native", Electro::Scripting::Electro_SpriteRendererComponent_SetColor);

        mono_add_internal_call("Electro.CameraComponent::IsPrimary_Native", Electro::Scripting::Electro_CameraComponent_IsPrimary);
        mono_add_internal_call("Electro.CameraComponent::SetAsPrimary_Native", Electro::Scripting::Electro_CameraComponent_SetAsPrimary);
        mono_add_internal_call("Electro.CameraComponent::IsFixedAspectRatio_Native", Electro::Scripting::Electro_CameraComponent_IsFixedAspectRatio);
        mono_add_internal_call("Electro.CameraComponent::SetFixedAspectRatio_Native", Electro::Scripting::Electro_CameraComponent_SetFixedAspectRatio);
    }
}