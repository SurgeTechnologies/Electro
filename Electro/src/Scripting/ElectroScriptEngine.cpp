//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroScriptEngine.hpp"
#include "ElectroMonoUtils.hpp"
#include "ElectroScriptRegistry.hpp"
#include "Scene/ElectroEntity.hpp"

namespace Electro
{
    static MonoDomain* sMonoDomain;

    static MonoAssembly* sCoreAssembly;
    static MonoAssembly* sAppAssembly;
    static Ref<Scene> sSceneContext;
    static MonoImage* sAppAssemblyImage  = nullptr;
    static MonoImage* sCoreAssemblyImage = nullptr;
    static const char* sAssemblyPath     = nullptr;
    static EntityInstanceMap sEntityInstanceMap;
    static std::unordered_map<String, MonoClass*> sClasses;
    static std::unordered_map<String, CSClass> sEntityClassMap;

    struct CSClass
    {
        String FullName;
        String ClassName;
        String NamespaceName;

        MonoClass* Class = nullptr;
        MonoMethod* Constructor = nullptr;
        MonoMethod* OnStartMethod = nullptr;
        MonoMethod* OnUpdateMethod = nullptr;

        void InitClassMethods(MonoImage* image)
        {
            Constructor    = Scripting::GetMethod(sCoreAssemblyImage, "Electro.ElectroAPI:.ctor(ulong)");
            OnStartMethod  = Scripting::GetMethod(image, FullName + ":OnStart()");
            OnUpdateMethod = Scripting::GetMethod(image, FullName + ":OnUpdate(single)");
        }
    };

    void ScriptEngine::Init(const char* assemblyPath)
    {
        mono_set_dirs("Electro/vendor/ElectroMono/lib", "Electro/vendor/ElectroMono/etc");
        auto domainX = mono_jit_init("Electro");

        char* name = (char*)"Electro-Runtime";
        sMonoDomain = mono_domain_create_appdomain(name, nullptr);

        LoadElectroRuntimeAssembly(assemblyPath);
    }

    void ScriptEngine::Shutdown()
    {
        mono_jit_cleanup(sMonoDomain);
        sSceneContext = nullptr;
        sEntityInstanceMap.clear();
    }

    void ScriptEngine::LoadElectroRuntimeAssembly(const String& path)
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

        MonoAssembly* appAssembly = Scripting::LoadAssembly(path.c_str());
        MonoImage* appAssemblyImage = Scripting::GetAssemblyImage(appAssembly);
        ScriptRegistry::RegisterAll();

        if (cleanup)
        {
            mono_domain_unload(sMonoDomain);
            sMonoDomain = domain;
        }

        sAppAssembly = appAssembly;
        sAppAssemblyImage = appAssemblyImage;
        sAssemblyPath = path.c_str();
    }

    const char* ScriptEngine::GetAssemblyPath()
    {
        return sAssemblyPath;
    }

    void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        sSceneContext = scene;
    }

    Ref<Scene> ScriptEngine::GetSceneContext()
    {
        E_ASSERT(sSceneContext, "Scene context is nullptr!");
        return sSceneContext;
    }

    bool ScriptEngine::IsEntityModuleValid(Entity entity)
    {
        return entity.HasComponent<ScriptComponent>() && ModuleExists(entity.GetComponent<ScriptComponent>().ModuleName);
    }

    void ScriptEngine::OnStart(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnStartMethod)
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnStartMethod);
    }

    void ScriptEngine::OnUpdate(Entity entity, Timestep ts)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnUpdateMethod)
        {
            void* args[] = { &ts };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnUpdateMethod, args);
        }
    }

    void ScriptEngine::InstantiateEntityClass(Entity entity)
    {
        Scene* scene = entity.mScene;
        UUID id = entity.GetComponent<IDComponent>().ID;
        auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

        EntityInstanceData& entityInstanceData = GetEntityInstanceData(scene->GetUUID(), id);
        EntityInstance& entityInstance = entityInstanceData.Instance;
        E_ASSERT(entityInstance.ScriptClass, "Invalid ScriptClass");

        entityInstance.Handle = InstantiateClass(*entityInstance.ScriptClass);

        void* param[] = { &id };
        Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->Constructor, param);
        OnStart(entity);
    }

    void ScriptEngine::InitScriptEntity(Entity entity)
    {
        Scene* scene = entity.mScene;
        UUID id = entity.GetComponent<IDComponent>().ID;
        auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

        if (moduleName == "ElectroNull")
            return;
        if (!ModuleExists(moduleName))
            return;

        CSClass& scriptClass = sEntityClassMap[moduleName];
        scriptClass.FullName = moduleName;
        if (moduleName.find('.') != String::npos)
        {
            scriptClass.NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
            scriptClass.ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
        }
        else
            scriptClass.ClassName = moduleName;

        scriptClass.Class = GetClass(sAppAssemblyImage, scriptClass);
        scriptClass.InitClassMethods(sAppAssemblyImage);

        EntityInstanceData& entityInstanceData = sEntityInstanceMap[scene->GetUUID()][id];
        EntityInstance& entityInstance = entityInstanceData.Instance;
        entityInstance.ScriptClass = &scriptClass;
    }

    void ScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
    {
        E_ASSERT(sEntityInstanceMap.find(sceneID) != sEntityInstanceMap.end(), "Entity not found!");
        auto& entityMap = sEntityInstanceMap.at(sceneID);
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Entity not found in EntityInstanceMap!");
        entityMap.erase(entityID);
    }

    MonoClass* ScriptEngine::GetClass(MonoImage* image, const CSClass& scriptClass)
    {
        MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
        if (!monoClass)
            ELECTRO_ERROR("Cannot find class in C#!");
        return monoClass;
    }

    Uint ScriptEngine::InstantiateClass(CSClass& scriptClass)
    {
        MonoObject* instance = mono_object_new(sMonoDomain, scriptClass.Class);
        if (!instance)
            ELECTRO_ERROR("Cannot instantiate C# class!");

        mono_runtime_object_init(instance);
        Uint handle = mono_gchandle_new(instance, false);
        return handle;
    
    }

    bool ScriptEngine::ModuleExists(const String& moduleName)
    {
        String NamespaceName;
        String ClassName;
        if (moduleName.find('.') != String::npos)
        {
            NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
            ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
        }
        else
            ClassName = moduleName;

        MonoClass* monoClass = mono_class_from_name(sAppAssemblyImage, NamespaceName.c_str(), ClassName.c_str());
        return monoClass != nullptr;
    }

    void ScriptEngine::OnSceneDestruct(UUID sceneID)
    {
        if (sEntityInstanceMap.find(sceneID) != sEntityInstanceMap.end())
        {
            sEntityInstanceMap.at(sceneID).clear();
            sEntityInstanceMap.erase(sceneID);
        }
    }

    void ScriptEngine::ReloadAssembly(const String& path)
    {
        LoadElectroRuntimeAssembly(path.c_str());
        if (sEntityInstanceMap.size())
        {
            Ref<Scene> scene = ScriptEngine::GetSceneContext();
            E_ASSERT(scene, "No active scene!");
            if (sEntityInstanceMap.find(scene->GetUUID()) != sEntityInstanceMap.end())
            {
                auto& entityMap = sEntityInstanceMap.at(scene->GetUUID());
                for (auto& [entityID, entityInstanceData] : entityMap)
                {
                    const auto& entityMap = scene->GetEntityMap();
                    E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
                    InitScriptEntity(entityMap.at(entityID));
                }
            }
        }
    }

    MonoObject* EntityInstance::GetInstance()
    {
        E_ASSERT(Handle, "Entity has not been instantiated!");
        return mono_gchandle_get_target(Handle);
    }

    EntityInstanceData& ScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
    {
        E_ASSERT(sEntityInstanceMap.find(sceneID) != sEntityInstanceMap.end(), "Invalid scene ID!");
        auto& entityIDMap = sEntityInstanceMap.at(sceneID);
        E_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
        return entityIDMap.at(entityID);
    }

    EntityInstanceMap& ScriptEngine::GetEntityInstanceMap()
    {
        return sEntityInstanceMap;
    }

    void ScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
    {
        //TODO
    }
}
