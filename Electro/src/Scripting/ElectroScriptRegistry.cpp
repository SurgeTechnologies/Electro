//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptRegistry.hpp"
#include <mono/metadata/object.h>

namespace Electro::Scripting
{
    String ConvertMonoStringToCppString(MonoString* message)
    {
        char* ptr = mono_string_to_utf8(message);
        String s{ ptr };
        mono_free(ptr);
        return s;
    }

    char* CovertMonoObjectToCppChar(MonoObject* obj)
    {
        if (obj == NULL)
        {
            char* a = "Null";
            return a;
        }
        else
        {
            MonoString* a = mono_object_to_string(obj, NULL);
            String b = ConvertMonoStringToCppString(a);
            char* s = _strdup(b.c_str());
            return s;
        }
    }

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
    void Electro_Console_LogCritical(MonoObject* message) {
        char* msg = CovertMonoObjectToCppChar(message);
        ELECTRO_CRITICAL(msg);
    }
}

namespace Electro
{
    void ScriptRegistry::RegisterAll()
    {
        mono_add_internal_call("Electro.Console::LogInfo_Native", Electro::Scripting::Electro_Console_LogInfo);
        mono_add_internal_call("Electro.Console::LogWarn_Native", Electro::Scripting::Electro_Console_LogWarn);
        mono_add_internal_call("Electro.Console::LogDebug_Native", Electro::Scripting::Electro_Console_LogDebug);
        mono_add_internal_call("Electro.Console::LogError_Native", Electro::Scripting::Electro_Console_LogError);
        mono_add_internal_call("Electro.Console::LogFatal_Native", Electro::Scripting::Electro_Console_LogCritical);
    }
}