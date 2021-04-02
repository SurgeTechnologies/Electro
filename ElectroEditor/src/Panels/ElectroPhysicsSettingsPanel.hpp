//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

namespace Electro
{
    class PhysicsSettingsWindow
    {
    public:
        static void OnImGuiRender(bool* show);

    private:
        static void RenderWorldSettings();
    };
}