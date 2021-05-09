//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Window.hpp"
#include "Renderer/GraphicsContext.hpp"
#include <Windows.h>

namespace Electro
{
    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowProps& props);

        virtual Uint GetHeight() override { return mData.Height; }
        virtual Uint GetWidth() override { return mData.Width; }
        virtual String& GetTitle() override { return mData.Title; }
        virtual void OnUpdate() override;
        virtual void* GetNativeWindow() override { return mWin32Window; }
        void SetEventCallback(const EventCallbackFn& callback) override { mData.EventCallback = callback; }
        void SetVSync(bool enabled) override { mData.VSync = enabled; }
        E_FORCE_INLINE bool IsVSync() const override { return mData.VSync; }
        virtual void Present() override;
        virtual void RegisterEditorModule(void* module) override { sEditorModule = module; }
        virtual void SetTitle(const String& title) override;
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
            bool VSync = true;
            EventCallbackFn EventCallback;
        };
        static void* sEditorModule;
        WindowData mData;
    };
}
