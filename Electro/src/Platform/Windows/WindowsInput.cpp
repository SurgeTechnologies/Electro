//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/Input.hpp"

namespace Electro
{
    bool Input::IsKeyPressed(const KeyCode keycode)
    {
        auto state = GetAsyncKeyState(static_cast<int>(keycode));
        return (state & 0x8000);
    }
    bool Input::IsMouseButtonPressed(const MouseCode button)
    {
        auto state = GetAsyncKeyState(static_cast<int>(button));
        return (state & 0x8000);
    }

    glm::vec2 Input::GetMousePosition()
    {
        POINT p;
        GetCursorPos(&p);
        return { (float)p.x, (float)p.y };
    }

    float Input::GetMouseX()
    {
        return GetMousePosition().x;
    }
    float Input::GetMouseY()
    {
        return GetMousePosition().y;
    }

    void Input::SetCursorMode(CursorMode cursorMode)
    {
        switch (cursorMode)
        {
            case CursorMode::NORMAL:
                SetCursor(LoadCursor(nullptr, IDC_ARROW)); break;
            case CursorMode::LOCKED:
                SetCursor(nullptr); break;
        }
    }
}
