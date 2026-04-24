//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#ifdef ELECTRO_PLATFORM_IOS

// Compiled as Objective-C++ (.mm) to access Foundation / UIKit APIs.
#import "Core/System/OS.hpp"
#import "Platform/iOS/iOSWindow.hpp"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace Electro
{
    // UIView set by the application delegate; used to create the window.
    static UIView* sRootView = nullptr;

    namespace iOSOSHelper
    {
        void SetRootView(UIView* view)
        {
            sRootView = view;
        }
    }

    Scope<Window> OS::CreateAppWindow(const WindowProps& props)
    {
        auto window = CreateScope<iOSWindow>(props);
        if (sRootView)
            window->SetUIView(sRootView);
        return window;
    }

    void OS::RunInTerminal(const char* /*cmd*/)
    {
        Log::Warn("OS::RunInTerminal is not supported on iOS.");
    }

    Uint OS::GetScreenWidth()
    {
        return static_cast<Uint>(UIScreen.mainScreen.bounds.size.width *
                                 UIScreen.mainScreen.scale);
    }

    Uint OS::GetScreenHeight()
    {
        return static_cast<Uint>(UIScreen.mainScreen.bounds.size.height *
                                 UIScreen.mainScreen.scale);
    }

    std::optional<String> OS::OpenFile(const char* /*filter*/)
    {
        // iOS has no traditional file-picker; use UIDocumentPickerViewController
        // via a JNI-equivalent bridge if needed.
        Log::Warn("OS::OpenFile is not supported on iOS.");
        return std::nullopt;
    }

    std::optional<String> OS::SaveFile(const char* /*filter*/)
    {
        Log::Warn("OS::SaveFile is not supported on iOS.");
        return std::nullopt;
    }

    char const* OS::SelectFolder(const String& /*title*/)
    {
        Log::Warn("OS::SelectFolder is not supported on iOS.");
        return nullptr;
    }

    void OS::SetClipboardText(const String& text)
    {
        NSString* nsText = [NSString stringWithUTF8String:text.c_str()];
        [UIPasteboard generalPasteboard].string = nsText;
    }

    void OS::OpenURL(const char* url)
    {
        NSURL* nsURL = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
        if (nsURL)
            [[UIApplication sharedApplication] openURL:nsURL
                                               options:@{}
                                     completionHandler:nil];
    }
}

// ── C-linkage bridge ──────────────────────────────────────────────────────────
// Called from the Objective-C main.mm entry point to hand over the root UIView
// before the engine is started.
extern "C" void iOSOSHelper_SetRootView(UIView* view)
{
    Electro::iOSOSHelper::SetRootView(view);
}

#endif // ELECTRO_PLATFORM_IOS
