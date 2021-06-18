//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Scene.hpp"

namespace YAML
{
    class Node;
    class Emitter;
}

namespace Electro
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const Ref<Scene>& scene, void* editorModule);

        void Serialize(const String& filepath);
        bool Deserialize(const String& filepath);
    private:
        void SerializeRendererSettings(YAML::Emitter& out);
        void DeserializeRendererSettings(YAML::Node& data);

        void SerializePhysicsSettings(YAML::Emitter& out);
        void DeserializePhysicsSettings(YAML::Node& data);

        // TODO: Remove
        void SerializeEditor(YAML::Emitter& out);
        void DeserializeEditor(YAML::Node& data);
    private:
        Ref<Scene> mScene;
        void* mEditorModuleContext;
    };
}
