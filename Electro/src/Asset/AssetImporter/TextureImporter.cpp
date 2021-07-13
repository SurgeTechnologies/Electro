//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "TextureImporter.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    bool TextureImporter::LoadData(AssetMetadata metaData, Ref<Asset>& asset)
    {
        Texture2DSpecification spec;
        spec.GenerateMips = true;
        spec.SRGB = false;
        spec.Path = (ProjectManager::GetAssetsDirectory() / metaData.Path).string();
        Ref<Texture2D> result = Texture2D::Create(spec);

        if (result->Loaded())
        {
            asset = result;
            return true;
        }

        return false;
    }

    bool TextureImporter::SaveData(AssetMetadata metaData, const Ref<Asset>& asset)
    {
        // TODO Implement
        return false;
    }
}
