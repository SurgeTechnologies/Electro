//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

namespace Electro
{
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
        static bool FileExists(const String& path);
        static bool Deletefile(const String& path);
        static bool Copyfile(const String& from, const String& to);
        static Vector<String> GetAllDirsInPath(const String& path);
        static Vector<String> GetAllFilePathsFromParentPath(const String& path);
        static Vector<char> ReadBinaryFile(const String& filepath);
        static Uint GetNumberOfFilesInDirectory(const String& directory); //Returns number of files in a directory (Including Folders)
        static const String& RenameFile(const String& path, const String& renameTo);
        static bool CreateOrEnsureFolderExists(const String& parentDirectory, const String& name);
        static bool CreateOrEnsureFolderExists(const String& directory);
        static bool IsDirectory(const String& path);
        static void RemoveAll(const String& fullpath);
    };
}
