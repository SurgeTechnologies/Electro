//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#ifdef ELECTRO_PLATFORM_IOS

#include "Core/Window.hpp"

// Forward-declare the Objective-C class so that the header remains valid
// from pure C++ translation units.
#ifdef __OBJC__
@class UIView;
@class CAMetalLayer;
#else
typedef struct objc_object UIView;
typedef struct objc_object CAMetalLayer;
#endif

namespace Electro
{
    // Window implementation backed by a UIView with a CAMetalLayer.
    // The UIView* is created by the Objective-C application delegate and
    // passed to SetUIView() before the window is used for rendering.
    class iOSWindow : public Window
    {
    public:
        explicit iOSWindow(const WindowProps& props);
        virtual ~iOSWindow();

        // Must be called from the Objective-C side (AppDelegate / ViewController)
        // after the UIView has been created and added to the view hierarchy.
        void SetUIView(UIView* view);

        // Window interface
        virtual void Present()  override;
        virtual void OnUpdate() override;

        virtual Uint GetWidth()  const override { return mData.Width;  }
        virtual Uint GetHeight() const override { return mData.Height; }

        virtual void SetEventCallback(const EventCallbackFn& cb) override { mData.EventCallback = cb; }
        virtual void SetVSync(bool enabled)  override { mData.VSync = enabled; }
        virtual void ShowConsole(bool)       override {} // no-op on mobile
        virtual void Minimize()              override {} // no-op on mobile
        virtual void Maximize()              override {} // no-op on mobile
        virtual bool IsVSync() const         override { return mData.VSync; }

        virtual void SetTitle(const String& title) override { mData.Title = title; }
        virtual const String& GetTitle() const     override { return mData.Title;  }

        virtual glm::vec2 GetPos()              const override { return { 0.0f, 0.0f }; }
        virtual void SetPos(const glm::vec2&)   const override {} // no-op on mobile

        virtual glm::vec2 GetSize()              const override;
        virtual void SetSize(const glm::vec2&)   const override;

        virtual void* GetNativeWindow() const override { return mUIView; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

        UIView*       mUIView      = nullptr;
        CAMetalLayer* mMetalLayer  = nullptr;

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

#endif // ELECTRO_PLATFORM_IOS
