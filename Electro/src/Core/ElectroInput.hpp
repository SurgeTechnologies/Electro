//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroKeyCodes.hpp"
#include "Core/ElectroMouseCodes.hpp"

namespace Electro
{
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseCode button);
        //static float* GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };
}