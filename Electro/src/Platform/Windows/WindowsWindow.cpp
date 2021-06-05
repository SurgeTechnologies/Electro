//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "WindowsWindow.hpp"
#include "Core/Application.hpp"
#include "Platform/DX11/DX11Context.hpp"
#include "EditorModule.hpp"
#include <windowsx.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Electro
{
    #define IDM_PROJECT_NEW              0
    #define IDM_PROJECT_OPEN             1
    #define IDM_SAVE                     3
    #define IDM_SAVE_AS                  4
    #define IDM_QUIT                     5
    #define IDM_SHOW_INSPECTOR_HIERARCHY 6
    #define IDM_SHOW_CONSOLE             7
    #define IDM_SHOW_VAULT               8
    #define IDM_SHOW_MATERIAL_INSPECTOR  9
    #define IDM_SHOW_PHYSICS_SETTINGS   10
    #define IDM_SHOW_RENDERER_SETTINGS  11
    #define IDM_SHOW_PROFILER           12

    HINSTANCE hInstance;
    void* WindowsWindow::sEditorModule;
    static bool sWin32Initialized;
    static int sWindowCreationBlocking = 0;

    WindowsWindow::WindowsWindow(const WindowProps& props)
    {
        Init(props);
    }

    void WindowsWindow::OnUpdate()
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        mContext->SwapBuffers();
    }

    void WindowsWindow::Init(const WindowProps& props)
    {
        mData.Height = props.Height;
        mData.Width = props.Width;
        mData.Title = props.Title;

        ELECTRO_INFO("Creating Window %s... (%ix%i)", mData.Title.c_str(), mData.Width, mData.Height);

        hInstance = GetModuleHandle(0);

        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = WindowProc;
        wc.style = CS_CLASSDC;
        wc.hInstance = hInstance;
        wc.lpszClassName = "Electro Win32Window";
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = (HICON)LoadImage(0, "Resources/Branding/ElectroMain.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        wc.hIconSm = wc.hIcon;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(WindowData*);

        if (!RegisterClassEx(&wc))
            ELECTRO_ERROR("Could not initialize the window class!");

        HMENU hMenubar = CreateMenu();
        HMENU hMenu = CreateMenu();
        HMENU hOtherMenu = CreateMenu();

        //File button
        AppendMenuW(hMenu, MF_STRING, IDM_PROJECT_NEW, L"&New Project");
        AppendMenuW(hMenu, MF_STRING, IDM_PROJECT_OPEN, L"&Open");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, IDM_SAVE, L"&Save");
        AppendMenuW(hMenu, MF_STRING, IDM_SAVE_AS, L"&Save As...");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, IDM_QUIT, L"&Exit Electro");
        AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&File");

        //View button
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_INSPECTOR_HIERARCHY, L"&Inspector and Hierarchy");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_CONSOLE, L"&Console");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_VAULT, L"&Vault");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_MATERIAL_INSPECTOR, L"&Material Inspector");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_PHYSICS_SETTINGS, L"&Physics Settings");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_RENDERER_SETTINGS, L"&Renderer Settings");
        AppendMenuW(hOtherMenu, MF_STRING, IDM_SHOW_PROFILER, L"&Profiler");
        AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hOtherMenu, L"&View");

        mWin32Window = CreateWindow(wc.lpszClassName, mData.Title.c_str(), WS_OVERLAPPEDWINDOW, 0, 0, mData.Width, mData.Height, NULL, hMenubar, wc.hInstance, NULL);

        if (!sWin32Initialized)
        {
            E_ASSERT(mWin32Window, "Could not initialize Win32!");
            sWin32Initialized = true;
        }
        SetWindowLongPtr(mWin32Window, 0, (LONG_PTR)&mData);

        mContext = CreateScope<DX11Context>(mWin32Window);
        mContext->Init();
    }

    void WindowsWindow::Present()
    {
        ShowWindow(mWin32Window, SW_SHOWDEFAULT);
        UpdateWindow(mWin32Window);
        SetFocus(mWin32Window);
    }

    void WindowsWindow::SetTitle(const String& title)
    {
        mData.Title = title;
        SetWindowText(mWin32Window, mData.Title.c_str());
    }

    LRESULT CALLBACK WindowsWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result = NULL;

        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_SIZE:
        {
            if (sWindowCreationBlocking > 0)
            {
                WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);
                data->Width = (UINT)LOWORD(lParam);
                data->Height = (UINT)HIWORD(lParam);

                WindowResizeEvent event((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
                data->EventCallback(event);
            }
            sWindowCreationBlocking++;
            break;
        }
        case WM_CLOSE:
        case WM_DESTROY:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            WindowCloseEvent event;
            data->EventCallback(event);
            break;
        }
        case WM_KEYUP:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            KeyReleasedEvent event(static_cast<KeyCode>(wParam));
            data->EventCallback(event);
            break;
        }
        case WM_CHAR:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            KeyTypedEvent event(static_cast<KeyCode>(wParam));
            data->EventCallback(event);
            break;
        }
        case WM_KEYDOWN:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);
            int repeatCount = (lParam & 0xffff);

            KeyPressedEvent event(static_cast<KeyCode>(wParam), repeatCount);
            data->EventCallback(event);
            break;
        }
        case WM_MOUSEMOVE:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseMovedEvent event((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
            data->EventCallback(event);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseScrolledEvent event((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
            data->EventCallback(event);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonPressedEvent event(static_cast<MouseCode>(VK_LBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_LBUTTONUP:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonReleasedEvent event(static_cast<MouseCode>(VK_LBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_MBUTTONDOWN:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonPressedEvent event(static_cast<MouseCode>(VK_MBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_MBUTTONUP:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonReleasedEvent event(static_cast<MouseCode>(VK_MBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonPressedEvent event(static_cast<MouseCode>(VK_RBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_RBUTTONUP:
        {
            WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, 0);

            MouseButtonReleasedEvent event(static_cast<MouseCode>(VK_RBUTTON));
            data->EventCallback(event);
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDM_PROJECT_NEW:
                    static_cast<EditorModule*>(sEditorModule)->NewProject(); break;
                case IDM_PROJECT_OPEN:
                    static_cast<EditorModule*>(sEditorModule)->Open(); break;
                case IDM_SAVE:
                    static_cast<EditorModule*>(sEditorModule)->SaveScene(); break;
                case IDM_SAVE_AS:
                    static_cast<EditorModule*>(sEditorModule)->SaveSceneAs(); break;
                case IDM_QUIT:
                    Application::Get().Close(); break;

                case IDM_SHOW_INSPECTOR_HIERARCHY:
                    static_cast<EditorModule*>(sEditorModule)->mShowHierarchyAndInspectorPanel = true; break;
                case IDM_SHOW_CONSOLE:
                    static_cast<EditorModule*>(sEditorModule)->mShowConsolePanel = true; break;
                case IDM_SHOW_VAULT:
                    static_cast<EditorModule*>(sEditorModule)->mShowVaultAndCachePanel = true; break;
                case IDM_SHOW_MATERIAL_INSPECTOR:
                    static_cast<EditorModule*>(sEditorModule)->mShowMaterialPanel = true; break;
                case IDM_SHOW_PHYSICS_SETTINGS:
                    static_cast<EditorModule*>(sEditorModule)->mShowPhysicsSettingsPanel = true; break;
                case IDM_SHOW_RENDERER_SETTINGS:
                    static_cast<EditorModule*>(sEditorModule)->mShowRendererSettingsPanel = true; break;
                case IDM_SHOW_PROFILER:
                    static_cast<EditorModule*>(sEditorModule)->mShowProfilerPanel = true; break;
            }
            break;
        }
        default:
            result = DefWindowProc(hWnd, msg, wParam, lParam);
        }

        return result;
    }
}
