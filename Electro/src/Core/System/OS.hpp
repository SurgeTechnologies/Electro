//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include <optional>

namespace Electro
{
    class Window;
    struct WindowProps;
    class OS
    {
    public:
        static Scope<Window> CreateAppWindow(const WindowProps& props);
        static Uint GetScreenWidth();
        static Uint GetScreenHeight();
        static void RunInTerminal(const char* cmd);

        static std::optional<String> E_NODISCARD OpenFile(const char* filter);
        static std::optional<String> E_NODISCARD SaveFile(const char* filter);
        static char const* SelectFolder(const String& title);
        static void OpenURL(const char* url);
    };
}
