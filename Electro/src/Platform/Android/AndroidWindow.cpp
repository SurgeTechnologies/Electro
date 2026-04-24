//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#ifdef ELECTRO_PLATFORM_ANDROID

#include "AndroidWindow.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "Core/Events/TouchEvent.hpp"
#include "Core/Log.hpp"

#include <android/log.h>

namespace Electro
{
    AndroidWindow::AndroidWindow(const WindowProps& props)
    {
        Init(props);
    }

    AndroidWindow::~AndroidWindow()
    {
        Shutdown();
    }

    void AndroidWindow::Init(const WindowProps& props)
    {
        mData.Title  = props.Title;
        mData.Width  = props.Width;
        mData.Height = props.Height;
        Log::Info("AndroidWindow: created ({0}x{1})", mData.Width, mData.Height);
        // EGL initialisation is deferred until SetNativeWindow() is called,
        // because the ANativeWindow is not available at construction time on
        // Android (it arrives via the onNativeWindowCreated callback).
    }

    void AndroidWindow::SetNativeWindow(ANativeWindow* window)
    {
        mNativeWindow = window;

        // ── EGL setup ──────────────────────────────────────────────────────────
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(mDisplay, nullptr, nullptr);

        // Choose a config suitable for OpenGL ES 3.x rendering
        const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
        };

        EGLConfig config;
        EGLint numConfigs;
        eglChooseConfig(mDisplay, attribs, &config, 1, &numConfigs);
        E_ASSERT(numConfigs > 0, "EGL: no suitable config found");

        mSurface = eglCreateWindowSurface(mDisplay, config, mNativeWindow, nullptr);

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribs);

        eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);

        // Query actual surface dimensions
        EGLint w = 0, h = 0;
        eglQuerySurface(mDisplay, mSurface, EGL_WIDTH,  &w);
        eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &h);
        mData.Width  = static_cast<Uint>(w);
        mData.Height = static_cast<Uint>(h);

        Log::Info("AndroidWindow: EGL surface ready ({0}x{1})", mData.Width, mData.Height);
    }

    void AndroidWindow::Shutdown()
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (mContext != EGL_NO_CONTEXT)
                eglDestroyContext(mDisplay, mContext);
            if (mSurface != EGL_NO_SURFACE)
                eglDestroySurface(mDisplay, mSurface);
            eglTerminate(mDisplay);
        }
        mDisplay      = EGL_NO_DISPLAY;
        mContext      = EGL_NO_CONTEXT;
        mSurface      = EGL_NO_SURFACE;
        mNativeWindow = nullptr;
    }

    void AndroidWindow::Present()
    {
        // The EGL surface is already current after SetNativeWindow(); nothing
        // extra is needed to "present" the window on Android.
    }

    void AndroidWindow::OnUpdate()
    {
        // Swap the back and front buffers.
        // Android input events are pumped by the NativeActivity glue in
        // android_main; they arrive via the AInputEvent callbacks, not here.
        if (mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE)
            eglSwapBuffers(mDisplay, mSurface);
    }

    void AndroidWindow::SetVSync(bool enabled)
    {
        mData.VSync = enabled;
        eglSwapInterval(mDisplay, enabled ? 1 : 0);
    }

    glm::vec2 AndroidWindow::GetSize() const
    {
        return { static_cast<float>(mData.Width), static_cast<float>(mData.Height) };
    }

    void AndroidWindow::SetSize(const glm::vec2& size) const
    {
        // The OS controls surface size on Android; this is a no-op.
        (void)size;
    }
}

#endif // ELECTRO_PLATFORM_ANDROID
