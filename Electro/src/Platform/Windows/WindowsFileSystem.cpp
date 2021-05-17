//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/FileSystem.hpp"

namespace Electro
{
    String FileSystem::GetNameWithoutExtension(const String& assetFilepath)
    {
        String name;
        auto lastSlash = assetFilepath.find_last_of("/\\");
        lastSlash = lastSlash == String::npos ? 0 : lastSlash + 1;
        auto lastDot = assetFilepath.rfind('.');
        auto count = lastDot == String::npos ? assetFilepath.size() - lastSlash : lastDot - lastSlash;
        name = assetFilepath.substr(lastSlash, count);
        return name;
    }

    String FileSystem::GetNameWithExtension(const String& assetFilepath)
    {
        return std::filesystem::path(assetFilepath).filename().string();
    }

    String FileSystem::GetExtension(const String& assetFilepath)
    {
        return std::filesystem::path(assetFilepath).extension().string();
    }

    String FileSystem::GetParentPath(const String& fullpath)
    {
        std::filesystem::path p = fullpath;
        return p.parent_path().string();
    }

    bool FileSystem::Deletefile(const String& path)
    {
        if (DeleteFile(path.c_str()) == FALSE)
        {
            ELECTRO_ERROR("Cannot delete file, invalid filepath %s", path);
            return false;
        }
        return true;
    }

    float FileSystem::GetFileSize(const String& path)
    {
        WIN32_FILE_ATTRIBUTE_DATA fad = {};
        if (!GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &fad))
        {
            ELECTRO_ERROR("Invalid filepath %s, cannot get the file size!", path);
            return -1;
        }

        LARGE_INTEGER size = {};
        size.HighPart = fad.nFileSizeHigh;
        size.LowPart = fad.nFileSizeLow;
        float mb = static_cast<float>(size.QuadPart / 1000000);
        return mb;
    }

    bool FileSystem::FileExists(const String& path)
    {
        DWORD dwAttrib = GetFileAttributes(path.c_str());
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool FileSystem::Copyfile(const String& from, const String& to)
    {
        if (CopyFile(from.c_str(), to.c_str(), FALSE) == FALSE)
        {
            ELECTRO_ERROR("Cannot copy file from %s to %s", from, to);
            return false;
        }

        FILETIME ft = {};
        SYSTEMTIME st = {};

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);
        HANDLE handle = CreateFile(to.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        bool f = SetFileTime(handle, (LPFILETIME)NULL, (LPFILETIME)NULL, &ft) != FALSE;
        E_ASSERT(f, "Internal Error");
        CloseHandle(handle);

        return true;
    }

    Vector<String> FileSystem::GetAllDirsInPath(const String& path)
    {
        Vector<String> paths;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    Vector<String> FileSystem::GetAllFilePathsFromParentPath(const String& path)
    {
        Vector<String> paths;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    bool FileSystem::CreateOrEnsureFolderExists(const String& parentDirectory, const String& name)
    {
        String path = String(parentDirectory) + "/" + String(name);
        if (std::filesystem::create_directory(path) || std::filesystem::exists(path))
            return true;
        return false;
    }

    bool FileSystem::CreateOrEnsureFolderExists(const String& directory)
    {
        if (std::filesystem::create_directory(directory) || std::filesystem::exists(directory))
            return true;
        return false;
    }

    String FileSystem::ReadFile(const String& filepath)
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
            ELECTRO_ERROR("Could not open file path \"%s\"", filepath.c_str());

        return result;
    }

    bool FileSystem::WriteFile(const String& filepath, const String& text)
    {
        std::ofstream out(filepath, std::ios::out);
        if (out.good())
        {
            out.write(text.c_str(), text.size());
            return true;
        }
        else
        {
            ELECTRO_ERROR("Could not open file path \"%s\"", filepath.c_str());
            return false;
        }
        return false;
    }

    Vector<char> FileSystem::ReadBinaryFile(const String& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream)
            ELECTRO_ERROR("Cannot open filepath: %s!", filepath);

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        size_t size = std::size_t(end - stream.tellg());
        if (size == 0) return {};

        Vector<char> buffer(size);
        if (!stream.read((char*)buffer.data(), buffer.size()))
            ELECTRO_ERROR("Cannot read file: %s", filepath);

        return buffer;
    }

    Uint FileSystem::GetNumberOfFilesInDirectory(const String& directory)
    {
        Uint fileCount = 0;
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory))
            if (entry.is_regular_file() || entry.is_directory())
                ++fileCount;

        return fileCount;
    }

    bool FileSystem::IsDirectory(const String& path)
    {
        return std::filesystem::is_directory(path);
    }

    void FileSystem::RemoveAll(const String& fullpath)
    {
        std::filesystem::remove_all(fullpath);
    }
}
