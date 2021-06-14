//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ScriptEngine.hpp"
#include "MonoUtils.hpp"
#include "ScriptRegistry.hpp"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <imgui.h>

namespace Electro
{
    static MonoDomain* sMonoDomain;
    static MonoAssembly* sCoreAssembly;
    static MonoAssembly* sAppAssembly;
    MonoImage* sAppAssemblyImage = nullptr;
    MonoImage* sCoreAssemblyImage = nullptr;
    static Ref<Scene> sSceneContext;
    static EntityInstanceMap sEntityInstanceMap;
    static std::unordered_map<String, CSClass> sEntityClassMap; // Mapped as <ClassName, Class>

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

            OnCollisionBeginMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnCollisionBegin(single)");
            OnCollisionEndMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnCollisionEnd(single)");
            OnTriggerBeginMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnTriggerBegin(single)");
            OnTriggerEndMethod = Scripting::GetMethod(sCoreAssemblyImage, "Electro.Entity:OnTriggerEnd(single)");
        }
    };

    void ScriptEngine::Init(const char* assemblyPath)
    {
        mono_set_dirs("Electro/vendor/ElectroMono/lib", "Electro/vendor/ElectroMono/etc");
        auto domainX = mono_jit_init("Electro");
        char* name = "Electro-Runtime";
        sMonoDomain = mono_domain_create_appdomain(name, nullptr);
        LoadElectroRuntimeAssembly(assemblyPath);
        ELECTRO_INFO("Initialized ScriptEngine");
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

        const String corePath = FileSystem::GetParentPath(path) + "/ElectroScript-Core.dll";

        sCoreAssembly = Scripting::LoadAssembly(corePath.c_str());
        sCoreAssemblyImage = Scripting::GetAssemblyImage(sCoreAssembly);

        sAppAssembly = Scripting::LoadAssembly(path.c_str());
        sAppAssemblyImage = Scripting::GetAssemblyImage(sAppAssembly);
        ScriptRegistry::RegisterAll();
        if (cleanup)
        {
            mono_domain_unload(sMonoDomain);
            sMonoDomain = domain;
        }
    }

    void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        sSceneContext = scene;
    }

    Ref<Scene> ScriptEngine::GetSceneContext()
    {
        return sSceneContext;
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

        // Set all public fields to appropriate values
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
        case MONO_TYPE_R4: return FieldType::Float;
        case MONO_TYPE_I4: return FieldType::Int;
        case MONO_TYPE_U4: return FieldType::UnsignedInt;
        case MONO_TYPE_STRING: return FieldType::_String;
        case MONO_TYPE_VALUETYPE:
        {
            char* name = mono_type_get_name(monoType);
            if (strcmp(name, "Electro.Vector2") == 0) return FieldType::Vec2;
            if (strcmp(name, "Electro.Vector3") == 0) return FieldType::Vec3;
            if (strcmp(name, "Electro.Vector4") == 0) return FieldType::Vec4;
        }
        }
        return FieldType::None;
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

        EntityInstanceData& entityInstanceData = sEntityInstanceMap[scene->GetUUID()][entityID];
        EntityInstance& entityInstance = entityInstanceData.Instance;
        entityInstance.ScriptClass = &scriptClass;

        // Fields
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
            void* ptr = 0;
            while ((iter = mono_class_get_fields(scriptClass.Class, &ptr)) != NULL)
            {
                const char* name = mono_field_get_name(iter);
                Uint flags = mono_field_get_flags(iter);
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
        E_ASSERT(sEntityInstanceMap.find(sceneID) != sEntityInstanceMap.end(), "Scene doesn't exist!");
        auto& entityMap = sEntityInstanceMap.at(sceneID);
        E_ASSERT(entityMap.find(entityID) != entityMap.end(), "Entity not found in EntityInstanceMap!");
        entityMap.erase(entityID);
    }

    void ScriptEngine::OnCollisionBegin(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnCollisionBeginMethod)
        {
            float value = 5.0f;
            void* args[] = { &value };
            Scripting::CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCollisionBeginMethod, args);
        }
    }

    void ScriptEngine::OnCollisionEnd(Entity entity)
    {
        EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
        if (entityInstance.ScriptClass->OnCollisionEndMethod)
        {
            float value = 5.0f;
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
            ELECTRO_ERROR("Cannot find class in C#!");
        return monoClass;
    }

    Uint ScriptEngine::InstantiateClass(CSClass& scriptClass)
    {
        MonoObject* instance = mono_object_new(sMonoDomain, scriptClass.Class);
        if (!instance)
            ELECTRO_ERROR("Cannot instantiate C# class!");

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
        if (sEntityInstanceMap.find(sceneID) != sEntityInstanceMap.end())
        {
            sEntityInstanceMap.at(sceneID).clear();
            sEntityInstanceMap.erase(sceneID);
        }
    }

    void ScriptEngine::ReloadAssembly(const String& path)
    {
        LoadElectroRuntimeAssembly(path);
        if (!sEntityInstanceMap.empty())
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

    MonoObject* EntityInstance::GetInstance() const
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
        // dst is the runtime scene, to which src is copied
        E_ASSERT(sEntityInstanceMap.find(dst) != sEntityInstanceMap.end(), "Scene is not in Instance Map");
        E_ASSERT(sEntityInstanceMap.find(src) != sEntityInstanceMap.end(), "Scene is not in Instance Map");

        std::unordered_map<UUID, EntityInstanceData>& dstEntityMap = sEntityInstanceMap.at(dst);
        std::unordered_map<UUID, EntityInstanceData>& srcEntityMap = sEntityInstanceMap.at(src);

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

        for (auto& [sceneID, entityMap] : sEntityInstanceMap)
        {
            bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
            if (opened)
            {
                Ref<Scene> scene = Scene::GetScene(sceneID);

                // Render all the entities in the entity map
                for (auto& [entityID, entityInstanceData] : entityMap)
                {
                    Entity entity = scene->GetScene(sceneID)->GetEntityMap().at(entityID);
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

// --------------------------------------------------------------------------------------
// -------------------------------------Public Field-------------------------------------
// --------------------------------------------------------------------------------------

    static Uint GetFieldSize(FieldType type)
    {
        switch (type)
        {
            case FieldType::Float:       return 4;
            case FieldType::Int:         return 4;
            case FieldType::UnsignedInt: return 4;
            //case FieldType::String:   return 8; // TODO
            case FieldType::Vec2:        return 4 * 2;
            case FieldType::Vec3:        return 4 * 3;
            case FieldType::Vec4:        return 4 * 4;
        }
        E_INTERNAL_ASSERT("Unknown field type!");
        return 0;
    }

    PublicField::PublicField(const std::string& name, const std::string& typeName, FieldType type)
        : mName(name), mTypeName(typeName), mType(type)
    {
        // Allocate enough memory for the given field type
        mStoredValueBuffer = AllocateBuffer(type);
    }

    PublicField::PublicField(PublicField&& other)
    {
        mName = std::move(other.mName);
        mTypeName = std::move(other.mTypeName);
        mType = other.mType;

        mEntityInstance = other.mEntityInstance;
        mMonoClassField = other.mMonoClassField;
        mStoredValueBuffer = other.mStoredValueBuffer;

        other.mEntityInstance = nullptr;
        other.mMonoClassField = nullptr;
        other.mStoredValueBuffer = nullptr;
    }

    PublicField::~PublicField()
    {
        delete[] mStoredValueBuffer;
    }

    void PublicField::CopyStoredValueToRuntime()
    {
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, mStoredValueBuffer);
    }

    bool PublicField::IsRuntimeAvailable() const
    {
        // mEntityInstance is only instantiated when runtime starts, 0 means not runtime
        return mEntityInstance->Handle != 0;
    }

    void PublicField::SetStoredValueRaw(void* src)
    {
        memcpy(mStoredValueBuffer, src, GetFieldSize(mType));
    }

    void PublicField::SetRuntimeValueRaw(void* src)
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, src);
    }

    void* PublicField::GetRuntimeValueRaw()
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        byte* outValue = nullptr;
        mono_field_get_value(mEntityInstance->GetInstance(), mMonoClassField, outValue);
        return outValue;
    }

    byte* PublicField::AllocateBuffer(FieldType type)
    {
        Uint size = GetFieldSize(type);
        byte* buffer = new byte[size];
        memset(buffer, 0, size);
        return buffer;
    }

    void PublicField::SetStoredValueInternal(void* value) const
    {
        memcpy(mStoredValueBuffer, value, GetFieldSize(mType));
    }

    void PublicField::GetStoredValueInternal(void* outValue) const
    {
        memcpy(outValue, mStoredValueBuffer, GetFieldSize(mType));
    }

    void PublicField::SetRuntimeValueInternal(void* value) const
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, value);
    }

    void PublicField::GetRuntimeValueInternal(void* outValue) const
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_get_value(mEntityInstance->GetInstance(), mMonoClassField, outValue);
    }
}
