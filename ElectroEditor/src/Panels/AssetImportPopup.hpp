//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#define TEXTURE_POPUP_KEY "Import Texture"

namespace Electro
{
    class AssetImportPopup
    {
    public:
        AssetImportPopup() = default;
        ~AssetImportPopup() = default;

        static void CatchTextureImportPopup();
        static void ThrowTextureImportPopup(const String& path);
    };
}