//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Fields.hpp"
#include "ScriptEngine.hpp"
#include <mono/metadata/object.h>

namespace Electro
{
    static Uint GetFieldSize(FieldType type)
    {
        switch (type)
        {
            case FieldType::FLOAT:       return 4;
            case FieldType::INT:         return 4;
            case FieldType::UINT: return 4;
            //case FieldType::String:   return 8; // TODO
            case FieldType::VEC2:        return 4 * 2;
            case FieldType::VEC3:        return 4 * 3;
            case FieldType::VEC4:        return 4 * 4;
        }
        E_INTERNAL_ASSERT("Unknown field type!");
        return 0;
    }

    PublicField::PublicField(const std::string& name, const std::string& typeName, FieldType type)
        : mName(name), mTypeName(typeName), mType(type)
    {
        // Allocate enough memory for the given field type
        mStoredValueBuffer = AllocateBuffer(type);
    }

    PublicField::PublicField(PublicField&& other) noexcept
    {
        mName = std::move(other.mName);
        mTypeName = std::move(other.mTypeName);
        mType = other.mType;

        mEntityInstance = other.mEntityInstance;
        mMonoClassField = other.mMonoClassField;
        mStoredValueBuffer = other.mStoredValueBuffer;

        other.mEntityInstance = nullptr;
        other.mMonoClassField = nullptr;
        other.mStoredValueBuffer = nullptr;
    }

    PublicField::~PublicField()
    {
        delete[] mStoredValueBuffer;
    }

    void PublicField::CopyStoredValueToRuntime()
    {
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, mStoredValueBuffer);
    }

    bool PublicField::IsRuntimeAvailable() const
    {
        // mEntityInstance is only instantiated when runtime starts, 0 means not runtime
        return mEntityInstance->Handle != 0;
    }

    void PublicField::SetStoredValueRaw(void* src)
    {
        memcpy(mStoredValueBuffer, src, GetFieldSize(mType));
    }

    void PublicField::SetRuntimeValueRaw(void* src)
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, src);
    }

    void* PublicField::GetRuntimeValueRaw()
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        byte* outValue = nullptr;
        mono_field_get_value(mEntityInstance->GetInstance(), mMonoClassField, outValue);
        return outValue;
    }

    byte* PublicField::AllocateBuffer(FieldType type)
    {
        Uint size = GetFieldSize(type);
        byte* buffer = new byte[size];
        memset(buffer, 0, size);
        return buffer;
    }

    void PublicField::SetStoredValueInternal(void* value) const
    {
        memcpy(mStoredValueBuffer, value, GetFieldSize(mType));
    }

    void PublicField::GetStoredValueInternal(void* outValue) const
    {
        memcpy(outValue, mStoredValueBuffer, GetFieldSize(mType));
    }

    void PublicField::SetRuntimeValueInternal(void* value) const
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_set_value(mEntityInstance->GetInstance(), mMonoClassField, value);
    }

    void PublicField::GetRuntimeValueInternal(void* outValue) const
    {
        E_ASSERT(mEntityInstance->GetInstance(), "Entity ScriptClass is invalid!");
        mono_field_get_value(mEntityInstance->GetInstance(), mMonoClassField, outValue);
    }
}
