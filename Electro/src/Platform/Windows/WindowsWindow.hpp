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
        virtual ~WindowsWindow();

        virtual void Present() override;
        virtual void OnUpdate() override;

        virtual Uint GetWidth() const override { return mData.Width; }
        virtual Uint GetHeight() const override { return mData.Height; }

        virtual void SetEventCallback(const EventCallbackFn& callback) override { mData.EventCallback = callback; }
        virtual void SetVSync(bool enabled) override { mData.VSync = enabled; }
        virtual void Minimize() override { ShowWindow(mWin32Window, SW_MINIMIZE); }
        virtual void Maximize() override { ShowWindow(mWin32Window, SW_MAXIMIZE); }
        virtual bool IsVSync() const override { return mData.VSync; }

        virtual void SetTitle(const String& title) override;
        virtual const String& GetTitle() const override { return mData.Title; }

        virtual glm::vec2 GetPos() const override;
        virtual void SetPos(const glm::vec2& pos) const override;

        virtual glm::vec2 GetSize() const override;
        virtual void SetSize(const glm::vec2& size) const override;

        virtual void* GetNativeWindow() const override { return mWin32Window; }
    private:
        virtual void Init(const WindowProps& props);
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
