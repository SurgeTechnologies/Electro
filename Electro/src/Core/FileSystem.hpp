//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

namespace Electro
{
    enum class FileFetchType
    {
        All = 0,
        ExcludingFolder
    };

    struct DirectoryEntry
    {
        DirectoryEntry() = default;
        DirectoryEntry(const String& pathInDisk);

        String Name;
        String Extension;
        String AbsolutePath;
        String ParentFolder;
        bool IsDirectory;

        virtual bool operator==(const DirectoryEntry& other) const { return AbsolutePath == other.AbsolutePath; }
        virtual bool operator!=(const DirectoryEntry& other) const { return !(*this == other); }
    };

    class FileSystem
    {
    public:
        static String GetNameWithoutExtension(const String& assetFilepath);
        static String GetNameWithExtension(const String& assetFilepath);
        static String GetExtension(const String& assetFilepath);
        static String GetParentPath(const String& fullpath);
        static String ReadFile(const String& filepath);
        static bool WriteFile(const String& filepath, const String& text);
        static float GetFileSize(const String& path);
        static bool Exists(const String& path);
        static bool Deletefile(const String& path);
        static bool Copyfile(const String& from, const String& to);
        static String EnsureExtension(char* name, const String& ext);
        static const Vector<String> GetAllDirsInPath(const String& path);
        static const Vector<String> GetAllFilePathsFromParentPath(const String& path);
        static const Vector<DirectoryEntry> GetFiles(const String& directory, FileFetchType fetchType = FileFetchType::All);
        static Vector<char> ReadBinaryFile(const String& filepath);
        static Uint GetNumberOfFilesInDirectory(const String& directory); //Returns number of files in a directory (Including Folders)
        static const String RenameFile(const String& path, const String& renameTo);

        static bool CreateOrEnsureFolderExists(const String& parentDirectory, const String& name);
        static bool CreateOrEnsureFolderExists(const String& directory);

        static bool IsDirectory(const String& path);
        static bool ValidatePath(const String& path);
        static void RemoveAll(const String& fullpath);
    };
}
