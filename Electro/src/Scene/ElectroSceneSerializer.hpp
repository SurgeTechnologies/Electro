//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroScene.hpp"
#include "ElectroEditorLayer.hpp"

namespace Electro
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const Ref<Scene>& scene, void* editorLayer);

        void Serialize(const String& filepath);
        bool Deserialize(const String& filepath);
    private:
        Ref<Scene> mScene;
        EditorLayer* mEditorLayerContext;
    };
}