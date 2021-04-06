//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroScene.hpp"
#include "ElectroEditorLayer.hpp"

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
        SceneSerializer(const Ref<Scene>& scene, void* editorLayer);

        void Serialize(const String& filepath);
        bool Deserialize(const String& filepath);
    private:
        void SerializeRendererSettings(YAML::Emitter& out);
        void DeserializeRendererSettings(YAML::Node& data);

        void SerializePhysicsSettings(YAML::Emitter& out);
        void DeserializePhysicsSettings(YAML::Node& data);
    private:
        Ref<Scene> mScene;
        EditorLayer* mEditorLayerContext;
    };
}