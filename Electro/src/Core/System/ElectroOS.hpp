//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    class OS
    {
    public:
        static String GetNameWithoutExtension(const String& assetFilepath);
        static String GetNameWithExtension(const char* assetFilepath);
        static String GetExtension(const char* assetFilepath);
        static Vector<String> GetAllDirsInPath(const char* path);
        static Vector<String> GetAllFilePathsFromParentPath(const char* path);
        static bool CreateFolder(const char* parentDirectory, const char* name);
        static Uint GetScreenWidth();
        static Uint GetScreenHeight();
        static String ReadFile(const char* filepath);
        static Vector<char> ReadBinaryFile(const char* filepath);
    };
}