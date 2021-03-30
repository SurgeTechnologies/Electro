//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptWrappers.hpp"
#include "ElectroMonoUtils.hpp"
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

    void Electro_Entity_CreateComponent(uint64_t entityID, void* type)
    {
        auto& entityMap = ValidateSceneAndReturnEntityMap(ScriptEngine::GetSceneContext(), entityID);
        Entity entity = entityMap.at(entityID);
        MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
        sCreateComponentFuncs[monoType](entity);
    }

    bool Electro_Entity_HasComponent(uint64_t entityID, void* type)
    {
        auto& entityMap = ValidateSceneAndReturnEntityMap(ScriptEngine::GetSceneContext(), entityID);
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

    MonoString* Electro_TagComponent_GetTag(uint64_t entityID)
    {
        auto& component = ValidateSceneAndReturnAComponent<TagComponent>(ScriptEngine::GetSceneContext(), entityID);
        String tag = component.Tag;
        return ConvertCppStringToMonoString(mono_domain_get(), tag.c_str());
    }

    void Electro_TagComponent_SetTag(uint64_t entityID, MonoString* inTag)
    {
        auto& component = ValidateSceneAndReturnAComponent<TagComponent>(ScriptEngine::GetSceneContext(), entityID);
        String& tag = ConvertMonoStringToCppString(inTag);
        component.Tag = tag;
    }

    void Electro_TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        *outTransform = component;
    }

    void Electro_TransformComponent_SetTransform(uint64_t entityID, TransformComponent* inTransform)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        component = *inTransform;
    }

    void Electro_TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        *outTranslation = component.Translation;
    }

    void Electro_TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.Translation = *inTranslation;
    }

    void Electro_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        *outRotation = component.Rotation;
    }

    void Electro_TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.Rotation = *inRotation;
    }

    void Electro_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        *outScale = component.Scale;
    }

    void Electro_TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale)
    {
        auto& component = ValidateSceneAndReturnAComponent<TransformComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.Scale = *inScale;
    }

    void Electro_SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor)
    {
        auto& component = ValidateSceneAndReturnAComponent<SpriteRendererComponent>(ScriptEngine::GetSceneContext(), entityID);
        *outColor = component.Color;
    }

    void Electro_SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor)
    {
        auto& component = ValidateSceneAndReturnAComponent<SpriteRendererComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.Color = *inColor;
    }

    void Electro_CameraComponent_SetAsPrimary(uint64_t entityID, bool* isPrimary)
    {
        auto& component = ValidateSceneAndReturnAComponent<CameraComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.Primary = isPrimary;
    }

    bool Electro_CameraComponent_IsPrimary(uint64_t entityID)
    {
        auto& component = ValidateSceneAndReturnAComponent<CameraComponent>(ScriptEngine::GetSceneContext(), entityID);
        return component.Primary;
    }

    void Electro_CameraComponent_SetFixedAspectRatio(uint64_t entityID, bool isAspectRatioFixed)
    {
        auto& component = ValidateSceneAndReturnAComponent<CameraComponent>(ScriptEngine::GetSceneContext(), entityID);
        component.FixedAspectRatio = isAspectRatioFixed;
    }

    bool Electro_CameraComponent_IsFixedAspectRatio(uint64_t entityID)
    {
        auto& component = ValidateSceneAndReturnAComponent<CameraComponent>(ScriptEngine::GetSceneContext(), entityID);
        return component.FixedAspectRatio;
    }
}
