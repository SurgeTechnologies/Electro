//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#ifdef ELECTRO_PLATFORM_IOS

// Compiled as Objective-C++ (.mm) for access to UIKit touch APIs.
#import "Core/Input.hpp"

#import <UIKit/UIKit.h>

namespace Electro
{
    // ── Internal touch state ───────────────────────────────────────────────────
    // Written by iOSInput::UpdateTouchState() (called from the ViewController's
    // UIResponder methods) and read by the public Input API.

    static constexpr int MAX_TOUCHES = 10;

    struct iOSTouchState
    {
        bool  active = false;
        float x      = 0.0f;
        float y      = 0.0f;
    };

    static iOSTouchState sTouches[MAX_TOUCHES];

    namespace iOSInput
    {
        // Called from the Objective-C UIResponder category for every UITouch
        // that begins, moves, or ends.
        void UpdateTouchState(int touchIndex, bool active, float x, float y)
        {
            if (touchIndex >= 0 && touchIndex < MAX_TOUCHES)
                sTouches[touchIndex] = { active, x, y };
        }
    }

    // ── Public Input interface ─────────────────────────────────────────────────
    bool Input::IsKeyPressed(const KeyCode /*keycode*/)
    {
        // iOS has no hardware keyboard in the general case; software keyboard
        // events arrive via UITextInput/UIKeyCommand which need a different
        // dispatch path.  Return false by default.
        return false;
    }

    bool Input::IsMouseButtonPressed(const MouseCode /*button*/)
    {
        // "Mouse button 0" maps to the primary touch pointer.
        return sTouches[0].active;
    }

    glm::vec2 Input::GetMousePosition()
    {
        return { sTouches[0].x, sTouches[0].y };
    }

    float Input::GetMouseX()
    {
        return sTouches[0].x;
    }

    float Input::GetMouseY()
    {
        return sTouches[0].y;
    }

    void Input::SetCursorMode(CursorMode /*cursorMode*/)
    {
        // iOS has no hardware cursor; this is a no-op.
    }
}

#endif // ELECTRO_PLATFORM_IOS
