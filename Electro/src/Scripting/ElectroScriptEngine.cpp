//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptEngine.hpp"
#include "ElectroMonoUtils.hpp"
#include "ElectroScriptRegistry.hpp"

namespace Electro
{
    static MonoDomain* sMonoDomain;

    static MonoAssembly* sCoreAssembly;
    static MonoAssembly* sAppAssembly;

    static MonoImage* sAppAssemblyImage = nullptr;
    static MonoImage* sCoreAssemblyImage = nullptr;

    void ScriptEngine::Init(const char* assemblyPath)
    {
        Scripting::InitMono(sMonoDomain);
        LoadElectroRuntimeAssembly(assemblyPath);
    }

    void ScriptEngine::Shutdown()
    {
        Scripting::ShutdownMono(sMonoDomain);
    }

    void ScriptEngine::LoadElectroRuntimeAssembly(const char* path)
    {
        MonoDomain* domain = nullptr;
        bool cleanup = false;
        if (sMonoDomain)
        {
            domain = mono_domain_create_appdomain("Electro Runtime", nullptr);
            mono_domain_set(domain, false);
            cleanup = true;
        }

        std::filesystem::path dllPath = path;
        String corePath = dllPath.parent_path().string() + "/ElectroScript-Core.dll";

        sCoreAssembly = Scripting::LoadAssembly(corePath.c_str());
        sCoreAssemblyImage = Scripting::GetAssemblyImage(sCoreAssembly);

        MonoAssembly* appAssembly = Scripting::LoadAssembly(path);
        MonoImage* appAssemblyImage = Scripting::GetAssemblyImage(appAssembly);
        ScriptRegistry::RegisterAll();

        if (cleanup)
        {
            mono_domain_unload(sMonoDomain);
            sMonoDomain = domain;
        }

        sAppAssembly = appAssembly;
        sAppAssemblyImage = appAssemblyImage;
    }
}