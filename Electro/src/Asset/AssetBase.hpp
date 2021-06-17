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
        Material,
        PhysicsMaterial,
        Mesh
    };

    namespace Utils
    {
        static const char* AssetTypeToString(const AssetType e)
        {
            switch (e)
            {
                case AssetType::None: return "None";
                case AssetType::Texture2D: return "Texture2D";
                case AssetType::EnvironmentMap: return "EnvironmentMap";
                case AssetType::Material: return "Material";
                case AssetType::PhysicsMaterial: return "PhysicsMaterial";
                case AssetType::Mesh: return "Mesh";
            }
            return "None";
        }
    }

    struct AssetHandle
    {
    public:
        UUID Handle;
        void MakeValid() { Handle = UUID(); }
        void MakeInvalid() { Handle = 0; }
        [[nodiscard]] bool IsValid() const { return Handle != 0; }

        bool operator==(const AssetHandle& other) const { return Handle == other.Handle; }
    };

    class Asset : public IElectroRef
    {
    public:
        virtual ~Asset() = default;
        E_FORCE_INLINE const String& GetName() const { return mName; }
        E_FORCE_INLINE const String& GetExtension() const { return mExtension; }
        E_FORCE_INLINE const String& GetPath() const { return mPathInDisk; }
        E_FORCE_INLINE AssetType GetType() const { return mBaseType; }
        E_FORCE_INLINE AssetHandle GetHandle() const { return mHandle; }

        virtual bool Serialize() { ELECTRO_WARN("No Serialization system avalilable for %s [Cannot serialize %s]", Utils::AssetTypeToString(mBaseType), mName.c_str()); return false; }
        virtual bool Deserialize() { ELECTRO_WARN("No Deserialization system avalilable for %s [Cannot deserialize %s]", Utils::AssetTypeToString(mBaseType), mName.c_str()); return false; }

        virtual bool operator==(const Asset& other) const { return mHandle == other.mHandle; }
        virtual bool operator!=(const Asset& other) const { return !(*this == other); }
    protected:
        String mName;
        String mPathInDisk;
        String mExtension;
        AssetType mBaseType = AssetType::None;
        AssetHandle mHandle;
    protected:
        void SetupAssetBase(const String& filepath, AssetType type, const String& name = "");
    };

    class PhysicsMaterial final : public Asset
    {
    public:
        PhysicsMaterial(const String& path);
        void Set(const glm::vec3& data);

        bool Serialize() override;
        bool Deserialize() override;

        static Ref<PhysicsMaterial> Create(const String& path);

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
