//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/UUID.hpp"

namespace Electro
{
    enum class AssetType
    {
        None,
        Texture2D,
        EnvironmentMap,
        Shader,
        PhysicsMaterial,
        Mesh
    };

    struct AssetHandle
    {
    public:
        UUID Handle;
        inline void MakeValid() { Handle = UUID(); }
        inline void MakeInvalid() { Handle = 0; }
        inline bool IsValid() const { return Handle != 0; }

        bool operator==(const AssetHandle& other) const { return Handle == other.Handle; }
    };

    class Asset : public IElectroRef
    {
    public:
        virtual ~Asset() {}
        const virtual String GetName() const { return mName; }
        const virtual String GetExtension() const { return mExtension; }
        const virtual String GetPath() const { return mPathInDisk; }

        virtual bool operator==(const Asset& other) const { return mHandle == other.mHandle; }
        virtual bool operator!=(const Asset& other) const { return !(*this == other); }
    public:
        String mName;
        String mPathInDisk;
        String mExtension;
        AssetType mBaseType;
        AssetHandle mHandle;
    protected:
        void SetupAssetBase(const String& filepath, AssetType type, const String& name = "");
    };

    class PhysicsMaterial : public Asset
    {
    public:
        PhysicsMaterial();
        PhysicsMaterial(const String& path);

        float mStaticFriction = 0.1f;
        float mDynamicFriction = 0.1f;
        float mBounciness = 0.1f;
    };
}

namespace std
{
    template <>
    struct hash<Electro::AssetHandle>
    {
        std::size_t operator()(const Electro::AssetHandle& handle) const
        {
            return hash<uint64_t>()((uint64_t)handle.Handle);
        }
    };
}
