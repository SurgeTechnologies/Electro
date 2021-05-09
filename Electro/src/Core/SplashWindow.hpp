//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Base.hpp"

namespace Electro
{
    struct SplashWindowProps
    {
        const char* Name;
        const char* ImagePath;
        int Width;
        int Height;
    };

    class SplashWindow
    {
    public:
        virtual ~SplashWindow() = default;
        virtual void Destroy() = 0;
        static Scope<SplashWindow> Create(const SplashWindowProps& props);
    };
}
