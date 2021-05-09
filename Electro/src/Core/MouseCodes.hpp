//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <cstdint>

namespace Electro
{
    using MouseCode = uint16_t;

    namespace Mouse
    {
        enum : MouseCode
        {
            ButtonLeft = 1,
            ButtonRight = 2,
            ButtonMiddle = 4
        };
    }
}