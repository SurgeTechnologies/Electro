//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#ifdef ELECTRO_PLATFORM_IOS

// This file is compiled as Objective-C++ (.mm) because it interacts with
// UIKit and QuartzCore (CAMetalLayer).
#import "iOSWindow.hpp"
#import "Core/Events/ApplicationEvent.hpp"
#import "Core/Events/TouchEvent.hpp"
#import "Core/Log.hpp"

#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Electro
{
    iOSWindow::iOSWindow(const WindowProps& props)
    {
        Init(props);
    }

    iOSWindow::~iOSWindow()
    {
        Shutdown();
    }

    void iOSWindow::Init(const WindowProps& props)
    {
        mData.Title  = props.Title;
        mData.Width  = props.Width;
        mData.Height = props.Height;
        Log::Info("iOSWindow: created ({0}x{1})", mData.Width, mData.Height);
        // The actual Metal layer is set up in SetUIView() once the UIView
        // arrives from the Objective-C application delegate.
    }

    void iOSWindow::SetUIView(UIView* view)
    {
        mUIView = view;

        // Set up a CAMetalLayer on the provided UIView
        mMetalLayer = [CAMetalLayer layer];
        mMetalLayer.pixelFormat      = MTLPixelFormatBGRA8Unorm;
        mMetalLayer.framebufferOnly  = YES;
        mMetalLayer.contentsScale    = UIScreen.mainScreen.scale;

        CGSize screenSize       = view.bounds.size;
        mMetalLayer.frame       = CGRectMake(0, 0, screenSize.width, screenSize.height);
        mMetalLayer.drawableSize = CGSizeMake(screenSize.width  * UIScreen.mainScreen.scale,
                                              screenSize.height * UIScreen.mainScreen.scale);

        mData.Width  = static_cast<Uint>(mMetalLayer.drawableSize.width);
        mData.Height = static_cast<Uint>(mMetalLayer.drawableSize.height);

        [view.layer addSublayer:mMetalLayer];

        Log::Info("iOSWindow: Metal layer ready ({0}x{1})", mData.Width, mData.Height);
    }

    void iOSWindow::Shutdown()
    {
        if (mMetalLayer)
        {
            [mMetalLayer removeFromSuperlayer];
            mMetalLayer = nullptr;
        }
        mUIView = nullptr;
    }

    void iOSWindow::Present()
    {
        // Metal drawable presentation is handled by the GraphicsContext
        // (Metal command buffer commit + presentDrawable).  Nothing needed here.
    }

    void iOSWindow::OnUpdate()
    {
        // On iOS the run loop is driven by UIKit's CADisplayLink or a manual
        // timer set up in the application delegate.  Input events arrive via
        // UIResponder overrides (touchesBegan:, touchesMoved:, touchesEnded:)
        // in the iOSViewController, which translates them into Electro events
        // and calls the EventCallback directly.
    }

    glm::vec2 iOSWindow::GetSize() const
    {
        return { static_cast<float>(mData.Width), static_cast<float>(mData.Height) };
    }

    void iOSWindow::SetSize(const glm::vec2& /*size*/) const
    {
        // Surface size is controlled by the OS on iOS; this is a no-op.
    }
}

#endif // ELECTRO_PLATFORM_IOS
