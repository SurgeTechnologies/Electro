//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroBase.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Core/ElectroRef.hpp"
#include "ElectroWindowsSplashWindow.hpp"

namespace Electro
{
    Scope<SplashWindow> SplashWindow::Create(const SplashWindowProps& props)
    {
        return CreateScope<WindowsSplashWindow>(props);
    }

    std::wstring StringToWideString(const String& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        std::wstring buf;
        buf.resize(len);
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength,
            const_cast<wchar_t*>(buf.c_str()), len);
        return buf;
    }

    WindowsSplashWindow::WindowsSplashWindow(const SplashWindowProps& props)
    {
        WNDCLASSEX windowClass = {};
        LPCSTR className = props.Name;
        HINSTANCE hInstance = GetModuleHandle(0);
        HANDLE iconRes = LoadImage(0, StringToWideString(props.ImagePath).c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
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
        windowClass.lpszClassName = StringToWideString(className).c_str();
        RegisterClassEx(&windowClass);

        int x = static_cast<int>((OS::GetScreenWidth()  - props.Width)  / 2);
        int y = static_cast<int>((OS::GetScreenHeight() - props.Height) / 2);

        mSplashWindow = CreateWindowEx(WS_EX_TOPMOST, StringToWideString(className).c_str(), StringToWideString(props.Name).c_str(), WS_POPUP, x, y, props.Width, props.Height, nullptr, nullptr, hInstance, nullptr);
        ShowWindow(mSplashWindow, true);

        RECT rect;
        GetClientRect(mSplashWindow, &rect);
        rect.right = rect.left + props.Width;
        rect.bottom = rect.top + props.Height;
        HDC hdc = GetDC(mSplashWindow);
        HBRUSH brush = CreatePatternBrush((HBITMAP)LoadImage(NULL, StringToWideString(props.ImagePath).c_str(), IMAGE_BITMAP, props.Width, props.Height, LR_DEFAULTSIZE | LR_LOADFROMFILE));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        ReleaseDC(mSplashWindow, hdc);
    }

    void WindowsSplashWindow::Destroy()
    {
        DestroyWindow(mSplashWindow);
    }
}
