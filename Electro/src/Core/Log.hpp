//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "fmt/color.h"

namespace Electro
{
    class Log
    {
    public:
        template <typename... Args>
        static void Trace(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::antique_white), format, args...);
            std::putc('\n', stdout);
        }

        template <typename... Args>
        static void Debug(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::aqua), format, args...);
            std::putc('\n', stdout);
        }

        template <typename... Args>
        static void Info(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::lawn_green), format, args...);
            std::putc('\n', stdout);
        }

        template <typename... Args>
        static void Warn(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::yellow), format, args...);
            std::putc('\n', stdout);
        }

        template <typename... Args>
        static void Error(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::red), format, args...);
            std::putc('\n', stdout);
        }

        template <typename... Args>
        static void Critical(const char* format, const Args&... args)
        {
            fmt::print(fg(fmt::color::antique_white) | bg(fmt::color::red), format, args...);
            std::putc('\n', stdout);
        }
    };
}
