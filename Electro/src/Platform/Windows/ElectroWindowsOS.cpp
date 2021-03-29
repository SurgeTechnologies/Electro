//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/System/ElectroOS.hpp"

#pragma warning(push, 0) //We all hate *garbage* warnings
#include <tinyfiledialogs.h>
#pragma warning(pop)

namespace Electro
{
    static const char* DialogTypeToString(DialogType type)
    {
        switch (type)
        {
        case DialogType::Ok:              return "ok";
        case DialogType::Ok__Cancel:      return "okcancel";
        case DialogType::Yes__No:         return "yesno";
        case DialogType::Yes__No__Cancel: return "yesnocancel";
        }
        ELECTRO_WARN("Invalid DialogType!");
        return "ok";
    }

    static const char* IconTypeToString(IconType type)
    {
        switch (type)
        {
        case IconType::Info:     return "info";
        case IconType::Warning:  return "warning";
        case IconType::Error:    return "error";
        case IconType::Question: return "question";
        }
        ELECTRO_WARN("Invalid IconType!");
        return "error";
    }

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

    int OS::AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton)
    {
        int result = tinyfd_messageBox(title.c_str(), message.c_str(), DialogTypeToString(dialogType), IconTypeToString(iconType), (int)defaultButton);
        return result;
    }

    char const* OS::InputBox(const String& title, const String& message)
    {
        char const* result = tinyfd_inputBox(title.c_str(), message.c_str(), "");
        return result;
    }

    char const* OS::PasswordBox(const String& title, const String& message)
    {
        char const* result = tinyfd_inputBox(title.c_str(), message.c_str(), 0);
        return result;
    }

    char const* OS::OpenFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc, bool allowMultipleSelects)
    {
        char const* result = tinyfd_openFileDialog(title.c_str(), defaultName.c_str(), numberOfFilters, filterPatterns, filterDesc.c_str(), (int)allowMultipleSelects);
        return result;
    }

    char const* OS::SaveFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc)
    {
        char const* result = tinyfd_saveFileDialog(title.c_str(), defaultName.c_str(), numberOfFilters, filterPatterns, filterDesc.c_str());
        return result;
    }

    char const* OS::SelectFolder(const String& title)
    {
        char const* result = tinyfd_selectFolderDialog(title.c_str(), "");
        return result;
    }
}