//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroInput.hpp"

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

    //kekw Input::GetMousePosition()
    //{
    //    POINT p;
    //
    //    GetCursorPos(&p);
    //
    //    return { (float)p.x, (float)p.y };
    //}

    float Input::GetMouseX()
    {
        POINT p;
        GetCursorPos(&p);
        return (float)p.x;
    }
    float Input::GetMouseY()
    {
        POINT p;
        GetCursorPos(&p);
        return (float)p.y;
    }
}