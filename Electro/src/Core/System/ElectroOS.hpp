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

    class Window;
    struct WindowProps;
    class OS
    {
    public:
        static Scope<Window> CreateAppWindow(const WindowProps& props);
        static String GetNameWithoutExtension(const String& assetFilepath);
        static String GetNameWithExtension(const char* assetFilepath);
        static String GetExtension(const char* assetFilepath);
        static String GetParentPath(const String& fullpath);
        static String ReadFile(const char* filepath);
        static void CopyToClipboard(const char* text);
        static float GetFileSize(const char* path);
        static bool FileExists(const char* path);
        static bool Deletefile(const char* path);
        static bool Copyfile(const char* from, const char* to);
        static void* Loadlibrary(const char* path);
        static void Unloadlibrary(void* handle);

        static Vector<String> GetAllDirsInPath(const char* path);
        static Vector<String> GetAllFilePathsFromParentPath(const char* path);
        static Vector<char> ReadBinaryFile(const char* filepath);

        static bool CreateFolder(const char* parentDirectory, const char* name);
        static bool CreateFolder(const char* directory);
        static bool IsDirectory(const String& path);

        static Uint GetScreenWidth();
        static Uint GetScreenHeight();

        static int AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton);
        static std::optional<String> E_NODISCARD OpenFile(const char* filter);
        static std::optional<String> E_NODISCARD SaveFile(const char* filter);
        static char const* SelectFolder(const String& title);
        static void OpenURL(const char* url);
        static void RemoveAll(const String& fullpath);
    };
}
