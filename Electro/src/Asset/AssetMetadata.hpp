//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <filesystem>
#include "AssetBase.hpp"

namespace Electro
{
    struct AssetMetadata
    {
        AssetHandle Handle = 0;
        AssetType Type;

        std::filesystem::path Path; // This path must be relative to the Assets Directory
        bool IsDataLoaded = false;

        bool IsValid() { return Handle != 0; }
    };
}