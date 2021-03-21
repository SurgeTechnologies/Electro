//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroSplashWindow.hpp"

namespace Electro
{
    class WindowsSplashWindow : public SplashWindow
    {
    public:
        WindowsSplashWindow(const SplashWindowProps& props);
        ~WindowsSplashWindow();
        virtual void Destroy() override;
    private:
        HWND mSplashWindow;
    };
}