//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IAssetEditor.hpp"
#include "Physics/PhysicsMaterial.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetImporter/AssetLoader.hpp"

namespace Electro
{
    class PhysicsMaterialEditor : public IAssetEditor
    {
    public:
        PhysicsMaterialEditor() = default;
        ~PhysicsMaterialEditor() = default;

        virtual void Render() override;
        virtual void SetForEdit(Ref<Asset>& asset) override;
    private:
        E_FORCE_INLINE void SerializePhysicsMaterial() { AssetLoader::Serialize(AssetManager::GetMetadata(mCurrentPhysicsMaterial->GetHandle()), Ref<Asset>(mCurrentPhysicsMaterial)); }
    private:
        PhysicsMaterial* mCurrentPhysicsMaterial;
        String mPhysicsMaterialNameBuffer;
    };
}