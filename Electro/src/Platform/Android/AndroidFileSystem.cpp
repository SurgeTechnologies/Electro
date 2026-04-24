//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#ifdef ELECTRO_PLATFORM_ANDROID

#include "Core/FileSystem.hpp"

#include <android/asset_manager.h>
#include <sys/stat.h>
#include <dirent.h>

// AAssetManager is used for read-only bundled assets (inside the APK).
// It must be set at startup via AndroidFileSystem::SetAssetManager().
namespace Electro
{
    static AAssetManager* sAssetManager = nullptr;

    namespace AndroidFileSystem
    {
        void SetAssetManager(AAssetManager* mgr)
        {
            sAssetManager = mgr;
        }
    }

    // ── Path helpers (pure C++ / STL – identical to Windows) ─────────────────

    String FileSystem::GetNameWithoutExtension(const String& assetFilepath)
    {
        auto lastSlash = assetFilepath.find_last_of("/\\");
        lastSlash = (lastSlash == String::npos) ? 0 : lastSlash + 1;
        auto lastDot  = assetFilepath.rfind('.');
        auto count    = (lastDot == String::npos) ? assetFilepath.size() - lastSlash : lastDot - lastSlash;
        return assetFilepath.substr(lastSlash, count);
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
        return std::filesystem::path(fullpath).parent_path().string();
    }

    // ── File deletion ─────────────────────────────────────────────────────────

    bool FileSystem::Deletefile(const String& path)
    {
        return (::remove(path.c_str()) == 0);
    }

    // ── File size ─────────────────────────────────────────────────────────────

    float FileSystem::GetFileSize(const String& path)
    {
        struct stat st = {};
        if (::stat(path.c_str(), &st) != 0)
        {
            Log::Error("Invalid filepath {0}, cannot get the file size!", path);
            return -1.0f;
        }
        return static_cast<float>(st.st_size) / 1'000'000.0f; // MB
    }

    // ── Existence check ───────────────────────────────────────────────────────

    bool FileSystem::Exists(const String& path)
    {
        struct stat st = {};
        return (::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
    }

    // ── Copy ──────────────────────────────────────────────────────────────────

    void FileSystem::Copy(const String& from, const String& to)
    {
        std::filesystem::copy(from, to,
            std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing);
    }

    // ── Extension helper ──────────────────────────────────────────────────────

    String FileSystem::EnsureExtension(char* name, const String& ext)
    {
        String nameString = name;
        const String extension = GetExtension(name);
        if (extension.empty())
            nameString.append(ext);
        return nameString;
    }

    // ── Directory enumeration ─────────────────────────────────────────────────

    const Vector<String> FileSystem::GetAllDirsInPath(const String& path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());
        return paths;
    }

    const Vector<String> FileSystem::GetAllFilePathsFromParentPath(const String& path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());
        return paths;
    }

    // ── Read / write (POSIX data-partition files) ─────────────────────────────

    String FileSystem::ReadFile(const String& filepath)
    {
        // First try the writable data partition
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            String result;
            in.seekg(0, std::ios::end);
            result.resize(static_cast<size_t>(in.tellg()));
            in.seekg(0, std::ios::beg);
            in.read(&result[0], static_cast<std::streamsize>(result.size()));
            return result;
        }

        // Fall back to bundled APK assets
        if (sAssetManager)
        {
            AAsset* asset = AAssetManager_open(sAssetManager, filepath.c_str(), AASSET_MODE_BUFFER);
            if (asset)
            {
                size_t size = static_cast<size_t>(AAsset_getLength(asset));
                String result(size, '\0');
                AAsset_read(asset, &result[0], size);
                AAsset_close(asset);
                return result;
            }
        }

        Log::Error("Could not open file path '{0}'", filepath);
        return {};
    }

    bool FileSystem::WriteFile(const String& filepath, const String& text)
    {
        std::ofstream out(filepath, std::ios::out);
        if (out.good())
        {
            out.write(text.c_str(), static_cast<std::streamsize>(text.size()));
            return true;
        }
        Log::Warn("Cannot open path for writing! - {0}", filepath);
        return false;
    }

    Vector<char> FileSystem::ReadBinaryFile(const String& filepath)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
        if (!stream)
        {
            // Try APK asset
            if (sAssetManager)
            {
                AAsset* asset = AAssetManager_open(sAssetManager, filepath.c_str(), AASSET_MODE_BUFFER);
                if (asset)
                {
                    size_t size = static_cast<size_t>(AAsset_getLength(asset));
                    Vector<char> buffer(size);
                    AAsset_read(asset, buffer.data(), size);
                    AAsset_close(asset);
                    return buffer;
                }
            }
            Log::Error("Cannot open filepath: {0}!", filepath);
            return {};
        }

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        size_t size = static_cast<size_t>(end - stream.tellg());
        if (size == 0)
            return {};

        Vector<char> buffer(size);
        if (!stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
            Log::Error("Cannot read file: {0}", filepath);
        return buffer;
    }

    Uint FileSystem::GetNumberOfFilesInDirectory(const String& directory)
    {
        Uint count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(directory))
            if (entry.is_regular_file() || entry.is_directory())
                ++count;
        return count;
    }

    const String FileSystem::RenameFile(const String& path, const String& renameTo)
    {
        std::filesystem::path p = path;
        String newFilePath = p.parent_path().string() + "/" + renameTo + p.extension().string();
        ::rename(path.c_str(), newFilePath.c_str());
        return newFilePath;
    }

    bool FileSystem::IsDirectory(const String& path)
    {
        return std::filesystem::is_directory(path);
    }

    bool FileSystem::ValidatePath(const String& path)
    {
        std::ifstream exists(path);
        return exists.is_open();
    }

    void FileSystem::RemoveAll(const String& fullpath)
    {
        std::filesystem::remove_all(fullpath);
    }

    bool FileSystem::CreateOrEnsureFolderExists(const String& parentDirectory, const String& name)
    {
        String path = parentDirectory + "/" + name;
        return (std::filesystem::create_directory(path) || std::filesystem::exists(path));
    }

    bool FileSystem::CreateOrEnsureFolderExists(const String& directory)
    {
        return (std::filesystem::create_directory(directory) || std::filesystem::exists(directory));
    }
}

#endif // ELECTRO_PLATFORM_ANDROID
