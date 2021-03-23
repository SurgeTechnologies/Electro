//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/System/ElectroOS.hpp"
#include <filesystem>

namespace Electro
{
    String OS::GetNameWithoutExtension(const String& assetFilepath)
    {
        String name;
        auto lastSlash = assetFilepath.find_last_of("/\\");
        lastSlash = lastSlash == String::npos ? 0 : lastSlash + 1;
        auto lastDot = assetFilepath.rfind('.');
        auto count = lastDot == String::npos ? assetFilepath.size() - lastSlash : lastDot - lastSlash;
        name = assetFilepath.substr(lastSlash, count);
        return name;
    }

    String OS::GetNameWithExtension(const char* assetFilepath)
    {
        return std::filesystem::path(assetFilepath).filename().string();
    }

    String OS::GetExtension(const char* assetFilepath)
    {
        return std::filesystem::path(assetFilepath).extension().string();
    }

    Vector<String> OS::GetAllDirsInPath(const char* path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    Vector<String> OS::GetAllFilePathsFromParentPath(const char* path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    bool OS::CreateFolder(const char* parentDirectory, const char* name)
    {
        String path = String(parentDirectory) + "/" + String(name);
        if (std::filesystem::create_directory(path) || std::filesystem::exists(path))
            return true;

        return false;
    }

    Uint OS::GetScreenWidth()
    {
        return GetSystemMetrics(SM_CXSCREEN);
    }

    Uint OS::GetScreenHeight()
    {
        return GetSystemMetrics(SM_CYSCREEN);
    }

    String OS::ReadFile(const char* filepath)
    {
        String result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
        }
        else
            ELECTRO_ERROR("Could not open file path \"%s\"", filepath);

        return result;
    }

    Vector<char> OS::ReadBinaryFile(const char* filepath)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream)
            ELECTRO_ERROR("Cannot open filepath: %s!", filepath);

        auto end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        auto size = std::size_t(end - stream.tellg());
        if (size == 0) return {};

        Vector<char> buffer(size);
        if (!stream.read((char*)buffer.data(), buffer.size()))
            ELECTRO_ERROR("Cannot read file: %s", filepath);

        return buffer;
    }
}