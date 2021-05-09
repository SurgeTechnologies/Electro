//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

namespace Electro
{
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID& other);

        operator uint64_t () { return mUUID; }
        operator const uint64_t() const { return mUUID; }
    private:
        uint64_t mUUID;
    };
}

namespace std
{
    template <>
    struct hash<Electro::UUID>
    {
        std::size_t operator()(const Electro::UUID& uuid) const
        {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}
