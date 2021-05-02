//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroSplashWindow.hpp"
#include <Windows.h>

namespace Electro
{
    class WindowsSplashWindow : public SplashWindow
    {
    public:
        WindowsSplashWindow(const SplashWindowProps& props);
        ~WindowsSplashWindow() = default;
        virtual void Destroy() override;
    private:
        HWND mSplashWindow;
    };
}
