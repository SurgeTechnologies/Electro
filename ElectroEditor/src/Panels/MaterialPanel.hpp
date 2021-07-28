//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Scene/Entity.hpp"
#include "UIMacros.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetImporter/AssetLoader.hpp"

namespace Electro
{
    class MaterialPanel : public IPanel
    {
    public:
        MaterialPanel() = default;
        ~MaterialPanel() = default;
        virtual void OnInit(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
        void SetMaterial(Ref<Material>& mat);

    private:
        E_FORCE_INLINE void SerializeMaterial() { AssetLoader::Serialize(AssetManager::GetMetadata(mCurrentMaterial->GetHandle()), Ref<Asset>(mCurrentMaterial)); }
    private:
        Material* mCurrentMaterial;
        char mMaterialNameBuffer[INPUT_BUFFER_LENGTH];
    };
}
