//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Core/UUID.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class AssetType
    {
        None,
        Texture2D,
        EnvironmentMap,
        Shader,
        Material,
        PhysicsMaterial,
        Mesh
    };

    static const char* AssetTypeToString(AssetType e)
    {
        switch (e)
        {
            case AssetType::None: return "None";
            case AssetType::Texture2D: return "Texture2D";
            case AssetType::EnvironmentMap: return "EnvironmentMap";
            case AssetType::Shader: return "Shader";
            case AssetType::Material: return "Material";
            case AssetType::PhysicsMaterial: return "PhysicsMaterial";
            case AssetType::Mesh: return "Mesh";
            default: return "unknown";
        }
    }

    struct AssetHandle
    {
    public:
        UUID Handle;
        void MakeValid() { Handle = UUID(); }
        void MakeInvalid() { Handle = 0; }
        bool IsValid() const { return Handle != 0; }

        bool operator==(const AssetHandle& other) const { return Handle == other.Handle; }
    };

    class Asset : public IElectroRef
    {
    public:
        virtual ~Asset() {}
        virtual String GetName() const { return mName; }
        virtual String GetExtension() const { return mExtension; }
        virtual String GetPath() const { return mPathInDisk; }
        virtual void Serialize() { ELECTRO_WARN("No Serialization system avalilable for %s [Cannot serialize %s]", AssetTypeToString(mBaseType), mName.c_str()); }
        virtual void Deserialize() { ELECTRO_WARN("No Deserialization system avalilable for %s [Cannot deserialize %s]", AssetTypeToString(mBaseType), mName.c_str()); }

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
        PhysicsMaterial(const String& path);
        void Set(const glm::vec3& data);

        void Serialize() override;
        void Deserialize() override;

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
            return hash<uint64_t>()(handle.Handle);
        }
    };
}
