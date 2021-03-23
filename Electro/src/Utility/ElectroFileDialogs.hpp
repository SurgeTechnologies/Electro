//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
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

    /* [Spike] Wrapper around the tinyfiledialogs [Spike] */
    class FileDialogs
    {
    public:
        FileDialogs() = default;
        ~FileDialogs() = default;

        static int AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton);
        static char const* InputBox(const String& title, const String& message);
        static char const* PasswordBox(const String& title, const String& message);
        static char const* OpenFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc, bool allowMultipleSelects);
        static char const* SaveFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc);
        static char const* SelectFolder(const String& title);
    private:
        static const char* DialogTypeToString(DialogType type);
        static const char* IconTypeToString(IconType type);
    };
}
