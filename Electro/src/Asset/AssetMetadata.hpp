//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <filesystem>
#include "AssetBase.hpp"

namespace Electro
{
    struct AssetMetadata
    {
        AssetHandle Handle = INVALID_ASSET_HANDLE;
        AssetType Type = AssetType::NONE;

        std::filesystem::path Path = ""; // This path must be relative to the Assets Directory
        bool IsDataLoaded = false;

        bool IsValid() { return Handle != INVALID_ASSET_HANDLE && Type != AssetType::NONE && Path != "" && IsDataLoaded != false; }
    };
}