//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "AssetMetadata.hpp"
#include <unordered_map>

namespace std
{
    template <>
    struct hash<std::filesystem::path>
    {
        std::size_t operator()(const std::filesystem::path& path) const
        {
            return hash_value(path);
        }
    };
}

namespace Electro
{
    class AssetRegistry
    {
    public:
        AssetRegistry() = default;

        bool Contains(const std::filesystem::path& path) const;
        size_t Remove(const std::filesystem::path& path);
        void Clear() { mRegistry.clear(); }

        const AssetMetadata& Get(const std::filesystem::path& path) const;
        AssetMetadata& operator[](const std::filesystem::path& path);

        std::unordered_map<std::filesystem::path, AssetMetadata>::iterator begin() { return mRegistry.begin(); }
        std::unordered_map<std::filesystem::path, AssetMetadata>::iterator end() { return mRegistry.end(); }
        std::unordered_map<std::filesystem::path, AssetMetadata>::const_iterator cbegin() { return mRegistry.cbegin(); }
        std::unordered_map<std::filesystem::path, AssetMetadata>::const_iterator cend() { return mRegistry.cend(); }
    private:
        std::unordered_map<std::filesystem::path, AssetMetadata> mRegistry;
    };
}
