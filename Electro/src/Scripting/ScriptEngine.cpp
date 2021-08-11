//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ScriptEngine.hpp"
#include "MonoUtils.hpp"
#include "ScriptRegistry.hpp"
#include "Scene/SceneManager.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <imgui.h>

namespace Electro
{
    // Images holds the the data inside the dlls
    MonoImage* sAppAssemblyImage = nullptr;
    MonoImage* sCoreAssemblyImage = nullptr;

    struct ScriptEngineData
    {
        MonoDomain* Domain;
        MonoAssembly* CoreAssembly;
        MonoAssembly* AppAssembly;
        Ref<Scene> SceneContext;
        Electro::EntityInstanceMap EntityInstanceMap;
        std::unordered_map<String, CSClass> EntityClassMap; // Mapped as <ClassName, Class>
    };

    static ScriptEngineData sData;

    struct CSClass
    {
        String FullName;
        String ClassName;
        String NamespaceName;

        MonoClass* Class = nullptr;
        MonoMethod* Constructor = nullptr;
        MonoMethod* OnStartMethod = nullptr;
        MonoMethod* OnUpdateMethod = nullptr;
        MonoMethod* OnFixedUpdateMethod = nullptr;

        MonoMethod* OnCollisionBeginMethod = nullptr;
        MonoMethod* OnCollisionEndMethod = nullptr;
        MonoMethod* OnTriggerBeginMethod = nullptr;
        MonoMethod* OnTriggerEndMethod = nullptr;

        void InitClassMethods(MonoImage* image)
        {
            Constructor = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:.ctor(ulong)");
            OnStartMethod = Scripting::GetMethod(image, FullName + ":OnStart()");
            OnUpdateMethod = Scripting::GetMethod(image, FullName + ":OnUpdate(single)");
            OnFixedUpdateMethod = Scripting::GetMethod(image, FullName + ":OnFixedUpdate(single)");

            OnCollisionBeginMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnCollisionBegin(ulong)");
            OnCollisionEndMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnCollisionEnd(ulong)");
            OnTriggerBeginMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnTriggerBegin(single)");
            OnTriggerEndMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnTriggerEnd(single)");
        }
    };

    void ScriptEngine::Init(const char* assemblyPath)
    {
        mono_set_dirs("Electro/vendor/ElectroMono/lib", "Electro/vendor/ElectroMono/etc");
        mono_jit_init("Electro");

        sData.Domain = Scripting::CreateDomain("Electro Runtime");;

        LoadAssemblies(assemblyPath);
        Log::Info("Initialized ScriptEngine");
    }

    void ScriptEngine::Shutdown()
    {
        mono_jit_cleanup(sData.Domain);
        sData.SceneContext = nullptr;
        sData.EntityInstanceMap.clear();
    }

    void ScriptEngine::LoadAssemblies(const String& path)
    {
        MonoDomain* domain = nullptr;
        bool cleanup = false;

        if (sData.Domain) // If domain already exists
        {
            domain = Scripting::CreateDomain("Electro Runtime");
            Scripting::SetDomain(domain);
            cleanup = true;
        }

        const String corePath = FileSystem::GetParentPath(path) + "/ElectroScript-Core.dll";

        sData.CoreAssembly = Scripting::LoadAssembly(corePath.c_str());
        sCoreAssemblyImage = Scripting::GetAssemblyImage(sData.CoreAssembly);

        sData.AppAssembly = Scripting::LoadAssembly(path.c_str());
        sAppAssemblyImage = Scripting::GetAssemblyImage(sData.AppAssembly);
        ScriptRegistry::RegisterAll();

        if (cleanup)
        {
            mono_domain_unload(sData.Domain);
            sData.Domain = domain;
        }
    }

    void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        sData.SceneContext = scene;
    }

    Ref<Scene> ScriptEngine::GetSceneContext()
    {
        return sData.SceneContext;
    }

    bool ScriptEngine::IsEntityModuleValid(Entity entity)
    {
        return entity.HasComponent<ScriptComponent>() && ModuleExists(entity.GetComponent<ScriptComponent>().ModuleName);
    }

    // Called when the runtime starts
    void ScriptEngine::InstantiateEntityClass(Entity entity)
    {
        Scene* scene = entity.mScene;
        UUID id = entity.GetComponent<IDComponent>().ID;
        const String& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

        EntityInstanceData& entityInstanceData = GetEntityInstanceData(scene->GetUUID(), id);
        EntityInstance& entityInstance = entityInstanceData.Instance;
        E_ASSERT(entityInstance.ScriptClass, "Invalid ScriptClass");

        entityInstance.Handle = InstantiateClass(*entityInstance.ScriptClass);

        // Set all public fields to appropriate values [Runtime]
        ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
        if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
        {
            auto& publicFields = moduleFieldMap.at(moduleName);
            for (auto& [name, field] : publicFields)
                field.CopyStoredValueToRuntime();
        }

        // Call the constructor in C#
        void* param[] = { &id };
        Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->Constructor, param);

        // Call the OnStart Method in C# (if any)
        OnStart(entity);
    }

    static FieldType GetElectroFieldType(MonoType* monoType)
    {
        int type = mono_type_get_type(monoType);
        switch (type)
        {
            case MONO_TYPE_R4: return FieldType::FLOAT;
            case MONO_TYPE_I4: return FieldType::INT;
            case MONO_TYPE_U4: return FieldType::UINT;
            case MONO_TYPE_STRING: return FieldType::STRING;
            case MONO_TYPE_VALUETYPE:
            {
                char* name = mono_type_get_name(monoType);
                if (strcmp(name, "Electro.Vector2") == 0) return FieldType::VEC2;
                if (strcmp(name, "Electro.Vector3") == 0) return FieldType::VEC3;
                if (strcmp(name, "Electro.Vector4") == 0) return FieldType::VEC4;
            }
        }
        return FieldType::NONE;
    }

    // Called when a ScriptComponent is added to an Entity
    void ScriptEngine::InitScriptEntity(Entity entity)
    {
        Scene* scene = entity.mScene;
        const UUID entityID = entity.GetComponent<IDComponent>().ID;
        const String& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

        if (moduleName == "ElectroNull")
            return;
        if (!ModuleExists(moduleName))
            return;

        CSClass& scriptClass = sData.EntityClassMap[moduleName];
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

        EntityInstanceData& entityInstanceData = sData.EntityInstanceMap[scene->GetUUID()][entityID];
        EntityInstance& entityInstance = entityInstanceData.Instance;
        entityInstance.ScriptClass = &scriptClass;

        // ----- Fields -----
        ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
        auto& fieldMap = moduleFieldMap[moduleName];

        // Save old fields
        std::unordered_map<String, PublicField> oldFields;
        oldFields.reserve(fieldMap.size());
        for (auto& [fieldName, field] : fieldMap)
            oldFields.emplace(fieldName, std::move(field));

        fieldMap.clear();

        {
            MonoClassField* iter;
            void* ptr = nullptr;
            while ((iter = mono_class_get_fields(scriptClass.Class, &ptr)) != nullptr)
            {
                const char* name = mono_field_get_name(iter);
                Uint flags = mono_field_get_flags(iter);

                // Skip the private varables
                if ((flags & MONO_FIELD_ATTR_PUBLIC) == 0)
                    continue;

                MonoType* fieldType = mono_field_get_type(iter);
                FieldType hazelFieldType = GetElectroFieldType(fieldType);

                // TODO: Attributes
                MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(scriptClass.Class, iter);

                char* typeName = mono_type_get_name(fieldType);

                if (oldFields.find(name) != oldFields.end())
                    fieldMap.emplace(name, std::move(oldFields.at(name)));
                else
                {
                    PublicField field = { name, typeName, hazelFieldType };
                    field.mEntityInstance = &entityInstance;
                    field.mMonoClassField = iter;
                    fieldMap.emplace(name, std::move(field));
                }
            }
        }
    }

    // Called when a ScriptComponent is removed
    void ScriptEngine::ShutdownScriptEntity(Entity entity, const String& moduleName)
    {
        EntityInstanceData& entityInstanceData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
        ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
        if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
            moduleFieldMap.erase(moduleName);
    }

    void ScriptEngine::OnStart(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnStartMethod)
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnStartMethod);
    }

    // Runs per Frame
    void ScriptEngine::OnUpdate(Entity entity, Timestep ts)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnUpdateMethod)
        {
            void* args[] = { &ts };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnUpdateMethod, args);
        }
    }

    void ScriptEngine::OnFixedUpdate(Entity entity, float fixedTimestep)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnFixedUpdateMethod)
        {
            void* args[] = { &fixedTimestep };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnFixedUpdateMethod, args);
        }
    }

    void ScriptEngine::OnScriptComponentDestroyed(UUID sceneID, const UUID entityID)
    {
        E_ASSERT(sData.EntityInstanceMap.find(sceneID) != sData.EntityInstanceMap.end(), "Scene doesn't exist!");
        auto& entityMap = sData.EntityInstanceMap.at(sceneID);
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Entity not found in EntityInstanceMap!");
        entityMap.erase(entityID);
    }

    void ScriptEngine::OnCollisionBegin(Entity entity, Entity other)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnCollisionBeginMethod)
        {
            uint64_t value = other.GetUUID();
            void* args[] = { &value };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCollisionBeginMethod, args);
        }
    }

    void ScriptEngine::OnCollisionEnd(Entity entity, Entity other)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnCollisionEndMethod)
        {
            uint64_t value = other.GetUUID();
            void* args[] = { &value };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCollisionEndMethod, args);
        }
    }

    void ScriptEngine::OnTriggerBegin(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnTriggerBeginMethod)
        {
            float value = 5.0f;
            void* args[] = { &value };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnTriggerBeginMethod, args);
        }
    }

    void ScriptEngine::OnTriggerEnd(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnTriggerEndMethod)
        {
            float value = 5.0f;
            void* args[] = { &value };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnTriggerEndMethod, args);
        }
    }

    MonoClass* ScriptEngine::GetClass(MonoImage* image, const CSClass& scriptClass)
    {
        MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
        if (!monoClass)
            Log::Error("Cannot find class in C#!");
        return monoClass;
    }

    Uint ScriptEngine::InstantiateClass(CSClass& scriptClass)
    {
        MonoObject* instance = mono_object_new(sData.Domain, scriptClass.Class);
        if (!instance)
            Log::Error("Cannot instantiate C# class!");

        mono_runtime_object_init(instance);
        const Uint handle = mono_gchandle_new(instance, false);
        return handle;
    }

    bool ScriptEngine::ModuleExists(const String& moduleName)
    {
        String namespaceName;
        String className;
        if (moduleName.find('.') != String::npos)
        {
            namespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
            className = moduleName.substr(moduleName.find_last_of('.') + 1);
        }
        else
            className = moduleName;

        MonoClass* monoClass = mono_class_from_name(sAppAssemblyImage, namespaceName.c_str(), className.c_str());
        return monoClass != nullptr;
    }

    void ScriptEngine::OnSceneDestruct(UUID sceneID)
    {
        if (sData.EntityInstanceMap.find(sceneID) != sData.EntityInstanceMap.end())
        {
            sData.EntityInstanceMap.at(sceneID).clear();
            sData.EntityInstanceMap.erase(sceneID);
        }
    }

    void ScriptEngine::ReloadAssembly(const String& path)
    {
        LoadAssemblies(path);

        if (!sData.EntityInstanceMap.empty())
        {
            Ref<Scene> scene = ScriptEngine::GetSceneContext();
            E_ASSERT(scene, "No active scene!");
            UUID sceneID = scene->GetUUID();
            if (sData.EntityInstanceMap.find(sceneID) != sData.EntityInstanceMap.end())
            {
                auto& entityMap = sData.EntityInstanceMap.at(sceneID);
                for (auto& [entityID, entityInstanceData] : entityMap)
                {
                    const auto& entityMap = scene->GetEntityMap();
                    E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
                    InitScriptEntity(entityMap.at(entityID));
                }
            }
        }
    }

    MonoObject* EntityInstance::GetInstance() const
    {
        E_ASSERT(Handle, "Entity has not been instantiated!");
        return mono_gchandle_get_target(Handle);
    }

    EntityInstanceData& ScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
    {
        E_ASSERT(sData.EntityInstanceMap.find(sceneID) != sData.EntityInstanceMap.end(), "Invalid scene ID!");
        auto& entityIDMap = sData.EntityInstanceMap.at(sceneID);
        E_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
        return entityIDMap.at(entityID);
    }

    EntityInstanceMap& ScriptEngine::GetEntityInstanceMap()
    {
        return sData.EntityInstanceMap;
    }

    void ScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
    {
        // dst is the runtime scene, to which src is copied
        E_ASSERT(sData.EntityInstanceMap.find(dst) != sData.EntityInstanceMap.end(), "Scene is not in Instance Map");
        E_ASSERT(sData.EntityInstanceMap.find(src) != sData.EntityInstanceMap.end(), "Scene is not in Instance Map");

        std::unordered_map<UUID, EntityInstanceData>& dstEntityMap = sData.EntityInstanceMap.at(dst);
        std::unordered_map<UUID, EntityInstanceData>& srcEntityMap = sData.EntityInstanceMap.at(src);

        for (auto& [entityID, entityInstanceData] : srcEntityMap)
        {
            for (auto& [moduleName, srcFieldMap] : srcEntityMap[entityID].ModuleFieldMap)
            {
                ScriptModuleFieldMap& dstModuleFieldMap = dstEntityMap[entityID].ModuleFieldMap;
                for (auto& [fieldName, field] : srcFieldMap)
                {
                    E_ASSERT(dstModuleFieldMap.find(moduleName) != dstModuleFieldMap.end(), "Module doesn't exist in destination Scene!");
                    std::unordered_map<String, PublicField>& fieldMap = dstModuleFieldMap.at(moduleName);
                    E_ASSERT(fieldMap.find(fieldName) != fieldMap.end(), "");
                    fieldMap.at(fieldName).SetStoredValueRaw(field.mStoredValueBuffer);
                }
            }
        }
    }

    void ScriptEngine::OnImGuiRender()
    {
        ImGui::Begin("ScriptEngine Debug");

        for (auto& [sceneID, entityMap] : sData.EntityInstanceMap)
        {
            bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
            if (opened)
            {
                Ref<Scene> scene = SceneManager::GetScene(sceneID);

                // Render all the entities in the entity map
                for (auto& [entityID, entityInstanceData] : entityMap)
                {
                    Entity entity = scene->GetEntityMap().at(entityID);
                    std::string entityName = "Unnamed Entity";
                    if (entity.HasComponent<TagComponent>())
                        entityName = entity.GetComponent<TagComponent>().Tag;
                    opened = ImGui::TreeNode((void*)(uint64_t)entityID, "%s (%llx)", entityName.c_str(), entityID);
                    if (opened)
                    {
                        // Render the module name associated with the entities
                        for (const auto& [moduleName, fieldMap] : entityInstanceData.ModuleFieldMap)
                        {
                            opened = ImGui::TreeNode(moduleName.c_str());
                            if (opened)
                            {
                                // Render all the fields in that module
                                for (const auto& [fieldName, field] : fieldMap)
                                {
                                    opened = ImGui::TreeNodeEx((void*)&field, ImGuiTreeNodeFlags_Leaf, fieldName.c_str());
                                    if (opened)
                                    {
                                        ImGui::TreePop();
                                    }
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
}
