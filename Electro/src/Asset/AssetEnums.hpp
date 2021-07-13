//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

namespace Electro
{
    enum class AssetFlag : uint16_t
    {
        NONE = 0,
        MISSING = E_BIT(0),
        INVALID = E_BIT(1)
    };

    enum class AssetType
    {
        NONE,
        SCENE,
        TEXTURE2D,
        ENVIRONMENT_MAP,
        MATERIAL,
        PHYSICS_MATERIAL
    };

    namespace Utils
    {
        inline AssetType AssetTypeFromString(const std::string& assetType)
        {
            if (assetType == "NONE")             return AssetType::NONE;
            if (assetType == "SCENE")            return AssetType::SCENE;
            if (assetType == "TEXTURE2D")        return AssetType::TEXTURE2D;
            if (assetType == "ENVIRONMENT_MAP")  return AssetType::ENVIRONMENT_MAP;
            if (assetType == "MATERIAL")         return AssetType::MATERIAL;
            if (assetType == "PHYSICS_MATERIAL") return AssetType::PHYSICS_MATERIAL;

            E_INTERNAL_ASSERT("Unknown Asset Type");
            return AssetType::NONE;
        }

        inline const char* AssetTypeToString(AssetType assetType)
        {
            switch (assetType)
            {
                case AssetType::NONE:             return "NONE";
                case AssetType::SCENE:            return "SCENE";
                case AssetType::TEXTURE2D:        return "TEXTURE2D";
                case AssetType::ENVIRONMENT_MAP:  return "ENVIRONMENT_MAP";
                case AssetType::MATERIAL:         return "MATERIAL";
                case AssetType::PHYSICS_MATERIAL: return "PHYSICS_MATERIAL";
            }

            E_INTERNAL_ASSERT("Unknown Asset Type");
            return "NONE";
        }
    }
}
