//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "AssetRegistry.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    static std::filesystem::path GetKey(const std::filesystem::path& path)
    {
        auto key = std::filesystem::relative(path, ProjectManager::GetAssetsDirectory());
        if (key.empty())
            key = path.lexically_normal();

        return key;
    }

    bool AssetRegistry::Contains(const std::filesystem::path& path) const
    {
        std::filesystem::path key = GetKey(path);
        return mRegistry.find(key) != mRegistry.end();
    }

    size_t AssetRegistry::Remove(const std::filesystem::path& path)
    {
        std::filesystem::path key = GetKey(path);
        return mRegistry.erase(key);
    }

    const AssetMetadata& AssetRegistry::Get(const std::filesystem::path& path) const
    {
        std::filesystem::path key = GetKey(path);
        return mRegistry.at(key);
    }

    AssetMetadata& AssetRegistry::operator[](const std::filesystem::path& path)
    {
        E_ASSERT(!path.string().empty(), "Path is empty!");
        std::filesystem::path key = GetKey(path);
        return mRegistry[key];
    }
}
