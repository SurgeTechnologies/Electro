//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroBase.hpp"
#include "Core/ElectroRef.hpp"
#include "ElectroWindowsSplashWindow.hpp"

namespace Electro
{
    Scope<SplashWindow> SplashWindow::Create(const SplashWindowProps& props)
    {
        return CreateScope<WindowsSplashWindow>(props);
    }


    WindowsSplashWindow::WindowsSplashWindow(const SplashWindowProps& props)
    {
        WNDCLASSEX windowClass = { 0 };
        LPCSTR className = props.Name;
        HINSTANCE hInstance = GetModuleHandle(0);
        HANDLE iconRes = LoadImage(0, props.ImagePath, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        windowClass.cbSize = sizeof(windowClass);
        windowClass.style = CS_OWNDC;
        windowClass.lpfnWndProc = DefWindowProc;
        windowClass.cbWndExtra = 0;
        windowClass.cbClsExtra = 0;
        windowClass.hInstance = hInstance;
        windowClass.hIcon = (HICON)iconRes;
        windowClass.hIconSm = (HICON)iconRes;
        windowClass.hCursor = nullptr;
        windowClass.hbrBackground = nullptr;
        windowClass.lpszClassName = className;
        RegisterClassEx(&windowClass);

        int x = (GetSystemMetrics(SM_CXSCREEN) - props.Width) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - props.Height) / 2;

        mSplashWindow = CreateWindowEx(WS_EX_TOPMOST, className, props.Name, WS_POPUP, x, y, props.Width, props.Height, nullptr, nullptr, hInstance, nullptr);
        ShowWindow(mSplashWindow, true);

        RECT rect;
        GetClientRect(mSplashWindow, &rect);
        rect.right = rect.left + props.Width;
        rect.bottom = rect.top + props.Height;
        HDC hdc = GetDC(mSplashWindow);
        HBRUSH brush = CreatePatternBrush((HBITMAP)LoadImage(NULL, props.ImagePath, IMAGE_BITMAP, props.Width, props.Height, LR_DEFAULTSIZE | LR_LOADFROMFILE));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        ReleaseDC(mSplashWindow, hdc);
    }

    WindowsSplashWindow::~WindowsSplashWindow()
    {
        DestroyWindow(mSplashWindow);
    }

    void WindowsSplashWindow::Destroy()
    {
        DestroyWindow(mSplashWindow);
    }
}
