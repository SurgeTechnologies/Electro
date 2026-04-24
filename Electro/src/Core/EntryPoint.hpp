//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

#if defined(ELECTRO_PLATFORM_WINDOWS)

extern Electro::Application* Electro::CreateApplication();

int main(int argc, char** argv)
{
    auto app = Electro::CreateApplication();
    app->Run();
    delete app;
}

#elif defined(ELECTRO_PLATFORM_ANDROID)

// Android entry point – invoked by the NativeActivity glue library.
// The android_app struct carries the ANativeActivity, ALooper, and input queue.
#include <android_native_app_glue.h>

extern Electro::Application* Electro::CreateApplication();

void android_main(struct android_app* state)
{
    // CreateApplication() should store the android_app pointer somewhere
    // accessible (e.g. via a platform-specific init call) before returning.
    // The pattern below follows the standard NDK NativeActivity flow.
    auto app = Electro::CreateApplication();
    app->Run();
    delete app;
}

#elif defined(ELECTRO_PLATFORM_IOS)

// iOS entry point – the @UIApplicationMain macro (or UIApplicationMain()) is
// called from an Objective-C stub in Platform/iOS/main.m.  The stub calls
// ElectroIOSMain() once the UIWindow is ready.

extern Electro::Application* Electro::CreateApplication();

// Called from the Objective-C application delegate (AppDelegate.mm) once
// UIKit has finished launching, so that the engine starts inside the run loop.
extern "C" void ElectroIOSMain()
{
    auto app = Electro::CreateApplication();
    app->Run();
    delete app;
}

#endif
