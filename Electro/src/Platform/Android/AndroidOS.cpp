//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#ifdef ELECTRO_PLATFORM_ANDROID

#include "Core/System/OS.hpp"
#include "Platform/Android/AndroidWindow.hpp"

#include <android/native_window.h>
#include <unistd.h>

namespace Electro
{
    // The ANativeWindow* set by the android_main entry point.
    // Set via AndroidOS::SetNativeWindow() before CreateAppWindow() is used.
    static ANativeWindow* sNativeWindow = nullptr;

    namespace AndroidOS
    {
        void SetNativeWindow(ANativeWindow* window)
        {
            sNativeWindow = window;
        }
    }

    Scope<Window> OS::CreateAppWindow(const WindowProps& props)
    {
        auto window = CreateScope<AndroidWindow>(props);
        if (sNativeWindow)
            window->SetNativeWindow(sNativeWindow);
        return window;
    }

    void OS::RunInTerminal(const char* cmd)
    {
        // system() is available but discouraged on Android; log the attempt.
        (void)cmd;
        Log::Warn("OS::RunInTerminal is not supported on Android.");
    }

    Uint OS::GetScreenWidth()
    {
        if (sNativeWindow)
            return static_cast<Uint>(ANativeWindow_getWidth(sNativeWindow));
        return 0;
    }

    Uint OS::GetScreenHeight()
    {
        if (sNativeWindow)
            return static_cast<Uint>(ANativeWindow_getHeight(sNativeWindow));
        return 0;
    }

    std::optional<String> OS::OpenFile(const char* /*filter*/)
    {
        // No native file-picker on Android; the app should use a custom UI.
        Log::Warn("OS::OpenFile is not supported on Android.");
        return std::nullopt;
    }

    std::optional<String> OS::SaveFile(const char* /*filter*/)
    {
        Log::Warn("OS::SaveFile is not supported on Android.");
        return std::nullopt;
    }

    char const* OS::SelectFolder(const String& /*title*/)
    {
        Log::Warn("OS::SelectFolder is not supported on Android.");
        return nullptr;
    }

    void OS::SetClipboardText(const String& /*text*/)
    {
        // Clipboard access on Android requires JNI / Java code.
        // Full implementation would call ClipboardManager via JNI.
        Log::Warn("OS::SetClipboardText is not yet implemented on Android.");
    }

    void OS::OpenURL(const char* url)
    {
        // Launching a URL requires an Intent, which in turn requires JNI.
        // Log for now; a full implementation would call startActivity via JNI.
        Log::Warn("OS::OpenURL not fully implemented on Android. URL: {0}", url);
    }
}

#endif // ELECTRO_PLATFORM_ANDROID
