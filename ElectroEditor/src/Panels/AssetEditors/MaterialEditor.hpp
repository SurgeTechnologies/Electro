//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Entity.hpp"
#include "UIMacros.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetImporter/AssetLoader.hpp"

namespace Electro
{
    class MaterialEditor
    {
    public:
        MaterialEditor() = default;
        ~MaterialEditor() = default;
        void SetMaterial(Ref<Material>& mat);

    private:
        void Render();
        E_FORCE_INLINE void SerializeMaterial() { AssetLoader::Serialize(AssetManager::GetMetadata(mCurrentMaterial->GetHandle()), Ref<Asset>(mCurrentMaterial)); }
    private:
        Material* mCurrentMaterial = nullptr;
        char mMaterialNameBuffer[INPUT_BUFFER_LENGTH] = {};
        friend class InspectorPanel;
    };
}
