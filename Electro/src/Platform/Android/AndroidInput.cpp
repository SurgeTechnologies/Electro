//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#ifdef ELECTRO_PLATFORM_ANDROID

#include "Core/Input.hpp"

// On Android there is no global polling API equivalent to Win32's
// GetAsyncKeyState.  Instead, key / touch state is tracked by the window
// layer via AInputEvent callbacks and stored in a simple state table.
// This file provides the cross-platform Input interface backed by that table.

namespace Electro
{
    // ── Internal state ─────────────────────────────────────────────────────────
    // These are written by the Android platform layer (AndroidWindow / the
    // android_main event loop) and read by the public Input API below.

    // Current touch pointer positions, indexed by pointer/finger ID (0–9).
    static constexpr int MAX_TOUCH_POINTERS = 10;

    struct TouchPointerState
    {
        bool  active = false;
        float x      = 0.0f;
        float y      = 0.0f;
    };

    static TouchPointerState sTouchPointers[MAX_TOUCH_POINTERS];

    // Key state (Android hardware keys like Back, Volume, etc.)
    static bool sKeyState[512] = {};

    // ── Internal helpers (called from the Android event loop) ─────────────────
    namespace AndroidInput
    {
        void SetKeyState(int keyCode, bool pressed)
        {
            if (keyCode >= 0 && keyCode < 512)
                sKeyState[keyCode] = pressed;
        }

        void SetPointerState(int pointerID, bool active, float x, float y)
        {
            if (pointerID >= 0 && pointerID < MAX_TOUCH_POINTERS)
                sTouchPointers[pointerID] = { active, x, y };
        }
    }

    // ── Public Input interface ─────────────────────────────────────────────────
    bool Input::IsKeyPressed(const KeyCode keycode)
    {
        int code = static_cast<int>(keycode);
        if (code >= 0 && code < 512)
            return sKeyState[code];
        return false;
    }

    bool Input::IsMouseButtonPressed(const MouseCode /*button*/)
    {
        // On Android, "mouse button 0" maps to the primary touch pointer.
        return sTouchPointers[0].active;
    }

    glm::vec2 Input::GetMousePosition()
    {
        return { sTouchPointers[0].x, sTouchPointers[0].y };
    }

    float Input::GetMouseX()
    {
        return sTouchPointers[0].x;
    }

    float Input::GetMouseY()
    {
        return sTouchPointers[0].y;
    }

    void Input::SetCursorMode(CursorMode /*cursorMode*/)
    {
        // Android has no hardware cursor; this is a no-op.
    }
}

#endif // ELECTRO_PLATFORM_ANDROID
