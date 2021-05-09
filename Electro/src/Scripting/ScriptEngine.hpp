//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Components.hpp"
#include "Scene/Entity.hpp"

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
        MonoObject* GetInstance();
    };

    struct EntityInstanceData
    {
        EntityInstance Instance;
        //ScriptModuleFieldMap ModuleFieldMap;
    };

    // Mapped as-> SceneID --- (EntityID - InstanceData)
    using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;

    class ScriptEngine
    {
    public:
        static void Init(const char* assemblyPath);
        static void Shutdown();
        static void LoadElectroRuntimeAssembly(const String& path);

        static void SetSceneContext(const Ref<Scene>& scene);
        static Ref<Scene> GetSceneContext();
        static bool IsEntityModuleValid(Entity entity);

        static void InstantiateEntityClass(Entity entity);
        static void InitScriptEntity(Entity entity);

        static void OnStart(Entity entity);
        static void OnUpdate(Entity entity, Timestep ts);
        static bool ModuleExists(const String& moduleName);
        static void OnSceneDestruct(UUID sceneID);
        static void ReloadAssembly(const String& path);
        static void OnScriptComponentDestroyed(UUID sceneID, UUID entityID);

        static void OnCollisionBegin(Entity entity);
        static void OnCollisionEnd(Entity entity);
        static void OnTriggerBegin(Entity entity);
        static void OnTriggerEnd(Entity entity);

        static EntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);
        static EntityInstanceMap& GetEntityInstanceMap();
        static void CopyEntityScriptData(UUID dst, UUID src);
    private:
        static MonoClass* GetClass(MonoImage* image, const CSClass& scriptClass);
        static Uint InstantiateClass(CSClass& scriptClass);
    };
}
