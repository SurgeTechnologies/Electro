//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
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
        static Uint GetScreenWidth();
        static Uint GetScreenHeight();
        static void RunInTerminal(const char* cmd);

        static int AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton);
        static std::optional<String> E_NODISCARD OpenFile(const char* filter);
        static std::optional<String> E_NODISCARD SaveFile(const char* filter);
        static char const* SelectFolder(const String& title);
        static void OpenURL(const char* url);
    };
}
