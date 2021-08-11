//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Components.hpp"
#include "Scene/Entity.hpp"
#include "Fields.hpp"

extern "C"
{
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
    typedef struct _MonoClass MonoClass;
}

namespace Electro
{
    struct CSClass;

    struct EntityInstance
    {
        CSClass* ScriptClass;
        Uint Handle = 0;
        Scene* SceneInstance = nullptr;
        MonoObject* GetInstance() const;
    };

    // Mapped as-> ModuleName --- (FieldName - PublicFieldData)
    using ScriptModuleFieldMap = std::unordered_map<String, std::unordered_map<String, PublicField>>;

    struct EntityInstanceData
    {
        EntityInstance Instance;
        ScriptModuleFieldMap ModuleFieldMap;
    };

    // Mapped as-> SceneID --- (EntityID - InstanceData)
    using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;

    class ScriptEngine
    {
    public:
        static void Init(const char* assemblyPath);
        static void Shutdown();
        static void LoadAssemblies(const String& path);

        static void SetSceneContext(const Ref<Scene>& scene);
        static Ref<Scene> GetSceneContext();

        static bool IsEntityModuleValid(Entity entity);
        static bool ModuleExists(const String& moduleName);

        static void InitScriptEntity(Entity entity);
        static void ShutdownScriptEntity(Entity entity, const String& moduleName);

        // C# methods
        static void OnStart(Entity entity);
        static void OnUpdate(Entity entity, Timestep ts);
        static void OnFixedUpdate(Entity entity, float fixedTimestep);
        static void OnCollisionBegin(Entity entity, Entity other);
        static void OnCollisionEnd(Entity entity, Entity other);
        static void OnTriggerBegin(Entity entity);
        static void OnTriggerEnd(Entity entity);

        static void InstantiateEntityClass(Entity entity);
        static void OnSceneDestruct(UUID sceneID);
        static void ReloadAssembly(const String& path);
        static void OnScriptComponentDestroyed(UUID sceneID, UUID entityID);

        static EntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);
        static EntityInstanceMap& GetEntityInstanceMap();
        static void CopyEntityScriptData(UUID dst, UUID src);

        static void OnImGuiRender(); //Debug Only
    private:
        static MonoClass* GetClass(MonoImage* image, const CSClass& scriptClass);
        static Uint InstantiateClass(CSClass& scriptClass);
    };
}
