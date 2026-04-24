//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#ifdef ELECTRO_PLATFORM_ANDROID

#include "Core/Window.hpp"
#include "Renderer/GraphicsContext.hpp"

// Android NDK headers
#include <android/native_window.h>
#include <EGL/egl.h>

namespace Electro
{
    // Window implementation backed by an ANativeWindow and an EGL surface.
    // The ANativeWindow is supplied by the NativeActivity / GameActivity
    // framework and must be set via SetNativeWindow() before Init() is called.
    class AndroidWindow : public Window
    {
    public:
        explicit AndroidWindow(const WindowProps& props);
        virtual ~AndroidWindow();

        // Must be called with the ANativeWindow* provided by the Android
        // framework (e.g. from android_main's android_app->window) before the
        // window is used for rendering.
        void SetNativeWindow(ANativeWindow* window);

        // Window interface
        virtual void Present()  override;
        virtual void OnUpdate() override;

        virtual Uint GetWidth()  const override { return mData.Width;  }
        virtual Uint GetHeight() const override { return mData.Height; }

        virtual void SetEventCallback(const EventCallbackFn& callback) override { mData.EventCallback = callback; }
        virtual void SetVSync(bool enabled) override;
        virtual void ShowConsole(bool /*show*/) override {}   // no-op on mobile
        virtual void Minimize()              override {}   // no-op on mobile
        virtual void Maximize()              override {}   // no-op on mobile
        virtual bool IsVSync()        const  override { return mData.VSync; }

        virtual void SetTitle(const String& title) override { mData.Title = title; }
        virtual const String& GetTitle() const     override { return mData.Title; }

        virtual glm::vec2 GetPos()  const          override { return { 0.0f, 0.0f }; }
        virtual void SetPos(const glm::vec2&) const override {}  // no-op on mobile

        virtual glm::vec2 GetSize()                  const override;
        virtual void SetSize(const glm::vec2& size)  const override;

        virtual void* GetNativeWindow() const override { return mNativeWindow; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

        ANativeWindow* mNativeWindow = nullptr;
        EGLDisplay     mDisplay      = EGL_NO_DISPLAY;
        EGLSurface     mSurface      = EGL_NO_SURFACE;
        EGLContext     mContext      = EGL_NO_CONTEXT;

        struct WindowData
        {
            String   Title;
            Uint     Width  = 0;
            Uint     Height = 0;
            bool     VSync  = true;
            EventCallbackFn EventCallback;
        };
        WindowData mData;
    };
}

#endif // ELECTRO_PLATFORM_ANDROID
