//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroUUID.hpp"
#include <random>

namespace Electro
{
    static std::random_device sRandomDevice;
    static std::mt19937_64 engine(sRandomDevice());
    static std::uniform_int_distribution<uint64_t> sUniformDistribution;

    UUID::UUID()
        : mUUID(sUniformDistribution(engine)){}

    UUID::UUID(uint64_t uuid)
        : mUUID(uuid) {}

    UUID::UUID(const UUID& other)
        : mUUID(other.mUUID) {}

}