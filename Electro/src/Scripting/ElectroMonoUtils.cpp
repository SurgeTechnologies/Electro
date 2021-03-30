//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroMonoUtils.hpp"
#include "Core/System/ElectroOS.hpp"
#include <mono/metadata/debug-helpers.h>

namespace Electro::Scripting
{
    void InitMono(MonoDomain* domain)
    {
        mono_set_dirs("Electro/vendor/ElectroMono/lib", "Electro/vendor/ElectroMono/etc");
        auto domainX = mono_jit_init("Electro");

        char* name = (char*)"Electro-Runtime";
        domain = mono_domain_create_appdomain(name, nullptr);
    }

    void ShutdownMono(MonoDomain* domain)
    {
        mono_jit_cleanup(domain);
    }

    MonoAssembly* LoadAssembly(const char* path)
    {
        Vector<char> fileData = OS::ReadBinaryFile(path);
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData.data(), fileData.size(), 1, &status, 0);
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
            ELECTRO_ERROR("Image not found from Assembly, maybe the Assembly is wrong ot corrupted? ElectroScriptEngine is not working!");
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
}