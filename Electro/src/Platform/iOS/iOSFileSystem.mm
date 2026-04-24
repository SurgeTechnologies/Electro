//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#ifdef ELECTRO_PLATFORM_IOS

// Compiled as Objective-C++ (.mm) to access Foundation / NSBundle APIs.
#import "Core/FileSystem.hpp"

#import <Foundation/Foundation.h>

#include <sys/stat.h>

// Utility: translate an Electro path to a writable full path on iOS.
// Reads first try the app's Documents directory, then fall back to the
// main bundle (read-only bundled resources).
static NSString* resolveReadPath(const Electro::String& path)
{
    // 1. Check absolute / already-rooted paths
    if (!path.empty() && path[0] == '/')
        return [NSString stringWithUTF8String:path.c_str()];

    // 2. Documents directory (writable user data)
    NSString* documents = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    NSString* candidate = [documents stringByAppendingPathComponent:[NSString stringWithUTF8String:path.c_str()]];
    if ([[NSFileManager defaultManager] fileExistsAtPath:candidate])
        return candidate;

    // 3. Main bundle (read-only resources shipped inside the .ipa)
    NSString* bundlePath = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:path.c_str()] ofType:nil];
    return bundlePath;
}

static NSString* resolveWritePath(const Electro::String& path)
{
    if (!path.empty() && path[0] == '/')
        return [NSString stringWithUTF8String:path.c_str()];

    NSString* documents = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    return [documents stringByAppendingPathComponent:[NSString stringWithUTF8String:path.c_str()]];
}

namespace Electro
{
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

    bool FileSystem::Deletefile(const String& path)
    {
        NSString* fullPath = resolveWritePath(path);
        NSError* error = nil;
        return [[NSFileManager defaultManager] removeItemAtPath:fullPath error:&error];
    }

    float FileSystem::GetFileSize(const String& path)
    {
        NSString* fullPath = resolveReadPath(path);
        if (!fullPath)
        {
            Log::Error("Invalid filepath {0}, cannot get the file size!", path);
            return -1.0f;
        }
        NSDictionary* attrs = [[NSFileManager defaultManager] attributesOfItemAtPath:fullPath error:nil];
        if (!attrs)
            return -1.0f;
        return static_cast<float>([attrs fileSize]) / 1'000'000.0f;
    }

    bool FileSystem::Exists(const String& path)
    {
        NSString* fullPath = resolveReadPath(path);
        if (!fullPath) return false;
        BOOL isDir = NO;
        return [[NSFileManager defaultManager] fileExistsAtPath:fullPath isDirectory:&isDir] && !isDir;
    }

    void FileSystem::Copy(const String& from, const String& to)
    {
        std::filesystem::copy(from, to,
            std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing);
    }

    String FileSystem::EnsureExtension(char* name, const String& ext)
    {
        String nameString = name;
        if (GetExtension(name).empty())
            nameString.append(ext);
        return nameString;
    }

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

    String FileSystem::ReadFile(const String& filepath)
    {
        NSString* fullPath = resolveReadPath(filepath);
        if (!fullPath)
        {
            Log::Error("Could not open file path '{0}'", filepath);
            return {};
        }
        NSData* data = [NSData dataWithContentsOfFile:fullPath];
        if (!data)
        {
            Log::Error("Could not read file path '{0}'", filepath);
            return {};
        }
        return String(reinterpret_cast<const char*>(data.bytes), data.length);
    }

    bool FileSystem::WriteFile(const String& filepath, const String& text)
    {
        NSString* fullPath = resolveWritePath(filepath);
        NSData* data = [NSData dataWithBytes:text.c_str() length:text.size()];
        NSError* error = nil;
        BOOL ok = [data writeToFile:fullPath options:NSDataWritingAtomic error:&error];
        if (!ok)
            Log::Warn("Cannot open path for writing! - {0}", filepath);
        return ok == YES;
    }

    Vector<char> FileSystem::ReadBinaryFile(const String& filepath)
    {
        NSString* fullPath = resolveReadPath(filepath);
        if (!fullPath)
        {
            Log::Error("Cannot open filepath: {0}!", filepath);
            return {};
        }
        NSData* data = [NSData dataWithContentsOfFile:fullPath];
        if (!data)
        {
            Log::Error("Cannot read file: {0}", filepath);
            return {};
        }
        const char* bytes = reinterpret_cast<const char*>(data.bytes);
        return Vector<char>(bytes, bytes + data.length);
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
        NSError* error = nil;
        [[NSFileManager defaultManager]
            moveItemAtPath:[NSString stringWithUTF8String:path.c_str()]
                    toPath:[NSString stringWithUTF8String:newFilePath.c_str()]
                     error:&error];
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
        String p = parentDirectory + "/" + name;
        return std::filesystem::create_directory(p) || std::filesystem::exists(p);
    }

    bool FileSystem::CreateOrEnsureFolderExists(const String& directory)
    {
        return std::filesystem::create_directory(directory) || std::filesystem::exists(directory);
    }
}

#endif // ELECTRO_PLATFORM_IOS
