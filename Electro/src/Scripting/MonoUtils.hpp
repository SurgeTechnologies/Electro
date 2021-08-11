//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scripting/ScriptEngine.hpp"
#include <mono/metadata/object.h>

extern "C"
{
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoAssembly MonoAssembly;
}

namespace Electro::Scripting
{
    MonoDomain* CreateDomain(char* friendlyName);
    void SetDomain(MonoDomain* domain);
    MonoAssembly* LoadAssembly(const char* path);
    MonoImage* GetAssemblyImage(MonoAssembly* assembly);

    // Function Utils
    MonoMethod* GetMethod(MonoImage* image, const String& methodName);
    MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr);

    // String Utils
    MonoString* ConvertCppStringToMonoString(MonoDomain* domain, const String& str);
    String ConvertMonoStringToCppString(MonoString* message);
    char* CovertMonoObjectToCppChar(MonoObject* obj);
}
