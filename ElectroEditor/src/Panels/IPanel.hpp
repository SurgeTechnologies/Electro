//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

namespace Electro
{
    class IPanel
    {
    public:
        IPanel() = default;
        virtual ~IPanel() = default;

        virtual void OnInit(void* data = nullptr) = 0;
        virtual void OnImGuiRender(bool* show) = 0;
    };
}