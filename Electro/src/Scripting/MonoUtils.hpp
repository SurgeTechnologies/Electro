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
    MonoAssembly* LoadAssembly(const char* path);
    MonoImage* GetAssemblyImage(MonoAssembly* assembly);
    MonoMethod* GetMethod(MonoImage* image, const String& methodName);
    MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr);
    MonoString* ConvertCppStringToMonoString(MonoDomain* domain, const String& str);
    String ConvertMonoStringToCppString(MonoString* message);
    char* CovertMonoObjectToCppChar(MonoObject* obj);
}
