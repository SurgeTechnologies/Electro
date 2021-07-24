//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IAssetLoader.hpp"

namespace YAML
{
    class Emitter;
}

namespace Electro
{
    class Material;
    class MaterialLoader : public IAssetLoader
    {
    public:
        virtual bool LoadData(AssetMetadata metaData, Ref<Asset>& asset) override;
        virtual bool SaveData(AssetMetadata metaData, const Ref<Asset>& asset) override;
    private:
        void SerializeTexture(Ref<Material>& mat, const char* mapName, YAML::Emitter& out);
    };
}