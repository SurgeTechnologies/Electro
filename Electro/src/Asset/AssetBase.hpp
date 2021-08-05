//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/UUID.hpp"
#include "AssetEnums.hpp"
#include <glm/glm.hpp>
#define INVALID_ASSET_HANDLE 0

namespace Electro
{
    using AssetHandle = UUID;
    E_FORCE_INLINE bool IsAssetHandleValid(const AssetHandle& handle) { return handle != INVALID_ASSET_HANDLE; }

    class Asset : public IElectroRef
    {
    public:
        virtual ~Asset() = default;

        // Gets the Asset Type
        const AssetType& GetType() const { return mBaseType; }

        // Sets the Asset Type
        void SetType(const AssetType& type) { mBaseType = type; }

        // Get the flags in this Asset
        const AssetFlag& GetFlags() const { return static_cast<AssetFlag>(mFlags); }

        // Set the flag(s) of this Asset
        void SetFlag(AssetFlag flags) { mFlags = static_cast<uint16_t>(flags); }

        // Gets the Underlying Handle, used by engine internally
        const AssetHandle& GetHandle() const { return mHandle; }

        // Sets the Underlying Handle, used by engine internally
        void SetHandle(AssetHandle handle) { mHandle = handle; }

        // Check if this Asset is valid or not
        bool IsValid() const { return ((mFlags & static_cast<uint16_t>(AssetFlag::MISSING)) | (mFlags & static_cast<uint16_t>(AssetFlag::INVALID))) == 0; }

        bool operator==(const Asset& other) const { return mHandle == other.mHandle; }
        bool operator!=(const Asset& other) const { return !(*this == other); }
    protected:
        AssetType mBaseType = AssetType::NONE;
        uint16_t mFlags = static_cast<uint16_t>(AssetFlag::NONE);
        AssetHandle mHandle = INVALID_ASSET_HANDLE;

        friend class AssetManager;
    };
}
