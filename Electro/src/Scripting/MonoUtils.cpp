//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "MonoUtils.hpp"
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

namespace Electro::Scripting
{
    MonoAssembly* LoadAssembly(const char* path)
    {
        Vector<char> fileData = FileSystem::ReadBinaryFile(path);
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData.data(), static_cast<uint32_t>(fileData.size()), 1, &status, 0);
        if (status != MONO_IMAGE_OK)
        {
            ELECTRO_CRITICAL("Bad MonoImage");
            return NULL;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, path, &status, 0);
        mono_image_close(image);

        if (!assembly)
            ELECTRO_CRITICAL("Could not load assembly: %s", path);

        return assembly;
    }

    MonoImage* GetAssemblyImage(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        if (!image)
            ELECTRO_ERROR("Image not found from Assembly, maybe the Assembly is wrong or corrupted? ElectroScriptEngine is not working!");
        return image;
    }

    MonoMethod* GetMethod(MonoImage* image, const String& methodName)
    {
        MonoMethodDesc* description = mono_method_desc_new(methodName.c_str(), NULL);
        if (!description)
            ELECTRO_ERROR("Method(Function) description creation failed!");

        MonoMethod* method = mono_method_desc_search_in_image(description, image);
        if (!method)
            ELECTRO_WARN("Method(Function) does not exist in image! [Invalid C# function name]");
        return method;
    }

    MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params)
    {
        MonoObject* pException = nullptr;
        MonoObject* result = mono_runtime_invoke(method, object, params, &pException);
        E_ASSERT(!pException, "Cannot call C# Method!");
        return result;
    }

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
            char* a = "NULL";
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

    MonoString* ConvertCppStringToMonoString(MonoDomain* domain, const String& str)
    {
        return mono_string_new(domain, str.c_str());
    }
}
