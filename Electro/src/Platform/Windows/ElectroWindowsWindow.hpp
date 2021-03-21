//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroWindow.hpp"
#include "Renderer/ElectroGraphicsContext.hpp"

namespace Electro
{
    class EWindowsWindow : public Window
    {
    public:
        EWindowsWindow(const WindowProps& props);

        virtual Uint GetHeight() override { return mData.Height; }
        virtual Uint GetWidth() override { return mData.Width; }
        virtual String& GetTitle() override { return mData.Title; }

        virtual void OnUpdate() override;
        void SetEventCallback(const EventCallbackFn& callback) override { mData.EventCallback = callback; }
        void SetVSync(bool enabled) override { mData.VSync = enabled; }
        bool IsVSync() const override { return mData.VSync; }
        virtual void Present() override;
    private:
        void Init(const WindowProps& props);
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    private:
        HWND mWin32Window;
        Scope<GraphicsContext> mContext;
        struct WindowData
        {
            String Title;
            unsigned int Width = 0, Height = 0;
            bool VSync = false;

            EventCallbackFn EventCallback;
        };

        WindowData mData;
    };
}