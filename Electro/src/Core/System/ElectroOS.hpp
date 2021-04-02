//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include <optional>

namespace Electro
{
    enum class DialogType
    {
        Ok = 0,
        Ok__Cancel,
        Yes__No,
        Yes__No__Cancel
    };

    enum class IconType
    {
        Info = 0,
        Warning,
        Error,
        Question
    };

    enum class DefaultButton
    {
        No = 0,
        Yes = 1,
        NO_in_YesNoCancel = 2
    };

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
        static int AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton);
        static std::optional<String> OpenFile(const char* filter);
        static std::optional<String> SaveFile(const char* filter);
        static char const* SelectFolder(const String& title);
    };
}