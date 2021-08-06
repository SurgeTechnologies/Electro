//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"

namespace Electro
{
    class IAssetEditor
    {
    public:
        virtual ~IAssetEditor() = default;

        virtual void SetForEdit(Ref<Asset>& asset) = 0;
        virtual void Render() = 0;
    };
}
