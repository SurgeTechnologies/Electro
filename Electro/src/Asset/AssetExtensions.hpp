//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "AssetEnums.hpp"
#include <unordered_map>

namespace Electro
{
    inline static std::unordered_map<String, AssetType> sAssetExtensionMap =
    {
        { ".electro", AssetType::SCENE            },
        { ".epm",     AssetType::PHYSICS_MATERIAL },
        { ".png",     AssetType::TEXTURE2D        },
        { ".jpg",     AssetType::TEXTURE2D        },
        { ".jpeg",    AssetType::TEXTURE2D        },
        { ".hdr",     AssetType::ENVIRONMENT_MAP  },
    };
}
