//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/Entity.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetImporter/AssetLoader.hpp"
#include "IAssetEditor.hpp"

namespace Electro
{
    class MaterialEditor : public IAssetEditor
    {
    public:
        MaterialEditor() = default;
        ~MaterialEditor() = default;

        virtual void SetForEdit(Ref<Asset>& asset) override;
        virtual void Render() override;

    private:
        E_FORCE_INLINE void SerializeMaterial() { AssetLoader::Serialize(AssetManager::GetMetadata(mCurrentMaterial->GetHandle()), Ref<Asset>(mCurrentMaterial)); }
    private:
        Material* mCurrentMaterial = nullptr;
        String mMaterialNameBuffer;
    };
}
