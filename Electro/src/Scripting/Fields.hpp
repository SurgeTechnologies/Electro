//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

extern "C"
{
    typedef struct _MonoClassField MonoClassField;
}

namespace Electro
{
    struct EntityInstance;
    enum class FieldType
    {
        NONE = 0, BOOL, INT, FLOAT, UINT, STRING, VEC2, VEC3, VEC4
    };

    class PublicField
    {
    public:
        String mName;
        String mTypeName;
        FieldType mType;

        PublicField(const std::string& name, const std::string& typeName, FieldType type);
        PublicField(const PublicField&) = delete;
        PublicField(PublicField&& other) noexcept;
        ~PublicField();

        void CopyStoredValueToRuntime();
        bool IsRuntimeAvailable() const;

        void SetStoredValueRaw(void* src);
        void* GetStoredValueRaw() { return mStoredValueBuffer; }

        template<typename T>
        T GetStoredValue() const
        {
            T value;
            GetStoredValueInternal(&value);
            return value;
        }

        template<typename T>
        void SetStoredValue(T value) const
        {
            SetStoredValueInternal(&value);
        }

        template<typename T>
        T GetRuntimeValue() const
        {
            T value;
            GetRuntimeValueInternal(&value);
            return value;
        }

        template<typename T>
        void SetRuntimeValue(T value) const
        {
            SetRuntimeValueInternal(&value);
        }

        void SetRuntimeValueRaw(void* src);
        void* GetRuntimeValueRaw();
    private:
        byte* AllocateBuffer(FieldType type);
        void SetStoredValueInternal(void* value) const;
        void GetStoredValueInternal(void* outValue) const;
        void SetRuntimeValueInternal(void* value) const;
        void GetRuntimeValueInternal(void* outValue) const;
    private:
        EntityInstance* mEntityInstance;
        MonoClassField* mMonoClassField;
        byte* mStoredValueBuffer = nullptr;

        friend class ScriptEngine;
    };
}