//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Asset/AssetEnums.hpp"

namespace Electro
{
    class AssetImportPopup
    {
    public:
        AssetImportPopup() = default;
        ~AssetImportPopup() = default;

        static void CatchImportPopup(const AssetType& type);
        static void ThrowImportPopup(const AssetType& type, const String& path);
    };
}