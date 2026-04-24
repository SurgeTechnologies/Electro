//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
//
// iOS entry point.  This file is an Objective-C stub that boots UIKit and then
// calls ElectroIOSMain() (declared in Core/EntryPoint.hpp) from the application
// delegate once the UIWindow is ready.
//
// Include this file in the Xcode project as the application's main.m.

#ifdef ELECTRO_PLATFORM_IOS

#import <UIKit/UIKit.h>

// Forward declaration (defined by Core/EntryPoint.hpp when included from C++)
extern void ElectroIOSMain(void);

// ── ElectroAppDelegate ────────────────────────────────────────────────────────
@interface ElectroAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@implementation ElectroAppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.backgroundColor = UIColor.blackColor;
    [self.window makeKeyAndVisible];

    // Hand the root UIView to the iOS platform layer so that
    // OS::CreateAppWindow() can attach the Metal layer to it.
    extern void iOSOSHelper_SetRootView(UIView*);
    iOSOSHelper_SetRootView(self.window.rootViewController.view ?: self.window);

    // Start the Electro engine on the main thread.
    // For production use, push this onto a display-link timer or a
    // background thread as appropriate for your game loop design.
    ElectroIOSMain();

    return YES;
}

@end

// ── Standard UIKit entry point ────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([ElectroAppDelegate class]));
    }
}

#endif // ELECTRO_PLATFORM_IOS
