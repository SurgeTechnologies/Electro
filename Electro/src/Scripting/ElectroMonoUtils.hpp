//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scripting/ElectroScriptEngine.hpp"
#include <mono/metadata/object.h>

extern "C"
{
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoAssembly MonoAssembly;
}

namespace Electro::Scripting
{
    MonoAssembly* LoadAssembly(const char* path);
    MonoImage* GetAssemblyImage(MonoAssembly* assembly);
    MonoMethod* GetMethod(MonoImage* image, const String& methodName);
    MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr);
    MonoString* ConvertCppStringToMonoString(MonoDomain* domain, const String& str);
    String ConvertMonoStringToCppString(MonoString* message);
    char* CovertMonoObjectToCppChar(MonoObject* obj);
    EntityMap ValidateSceneAndReturnEntityMap(Ref<Scene>& sceneContext, uint64_t entityID);

    template<typename T>
    T& ValidateSceneAndReturnAComponent(Ref<Scene>& sceneContext, uint64_t entityID)
    {
        auto& entityMap = ValidateSceneAndReturnEntityMap(ScriptEngine::GetSceneContext(), entityID);
        Entity entity = entityMap.at(entityID);
        E_ASSERT(entity.HasComponent<T>(), "");
        auto& component = entity.GetComponent<T>();
        return component;
    }
}