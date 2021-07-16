//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "Core/Base.hpp"

namespace Electro
{
    class PhysicsSettingsPanel : public IPanel
    {
    public:
        PhysicsSettingsPanel() = default;
        virtual void OnInit(void* data) {}
        virtual void OnImGuiRender(bool* show) override;
    };
}
