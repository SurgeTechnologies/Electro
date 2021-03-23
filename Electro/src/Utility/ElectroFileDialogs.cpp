//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroFileDialogs.hpp"
#include <tinyfiledialogs.h>

namespace Electro
{
    int FileDialogs::AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton)
    {
        int result;
        result = tinyfd_messageBox(title.c_str(), message.c_str(), DialogTypeToString(dialogType), IconTypeToString(iconType), (int)defaultButton);
        return result;
    }

    char const* FileDialogs::InputBox(const String& title, const String& message)
    {
        char const* result;
        result = tinyfd_inputBox(title.c_str(), message.c_str(), "");
        return result;
    }

    char const* FileDialogs::PasswordBox(const String& title, const String& message)
    {
        char const* result;
        result = tinyfd_inputBox(title.c_str(), message.c_str(), NULL);
        return result;
    }

    char const* FileDialogs::OpenFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc, bool allowMultipleSelects)
    {
        char const* result;
        result = tinyfd_openFileDialog(title.c_str(), defaultName.c_str(), numberOfFilters, filterPatterns, filterDesc.c_str(), (int)allowMultipleSelects);
        return result;
    }

    char const* FileDialogs::SaveFile(const String& title, const String& defaultName, const int numberOfFilters, char const* const* const filterPatterns, const String& filterDesc)
    {
        char const* result;
        result = tinyfd_saveFileDialog(title.c_str(), defaultName.c_str(), numberOfFilters, filterPatterns, filterDesc.c_str());
        return result;
    }

    char const* FileDialogs::SelectFolder(const String& title)
    {
        char const* result = tinyfd_selectFolderDialog(title.c_str(), "");
        return result;
    }

    const char* FileDialogs::DialogTypeToString(DialogType type)
    {
        switch (type)
        {
            case DialogType::Ok:              return "ok";
            case DialogType::Ok__Cancel:      return "okcancel";
            case DialogType::Yes__No:         return "yesno";
            case DialogType::Yes__No__Cancel: return "yesnocancel";
        }
        ELECTRO_WARN("Invalid DialogType!"); /* [Spike] Unreachable [Spike] */
        return "ok";
    }

    const char* FileDialogs::IconTypeToString(IconType type)
    {
        switch (type)
        {
            case IconType::Info:     return "info";
            case IconType::Warning:  return "warning";
            case IconType::Error:    return "error";
            case IconType::Question: return "question";
        }
        ELECTRO_WARN("Invalid IconType!"); /* [Spike] Unreachable [Spike] */
        return "error";
    }
}