//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    class PhysicsSettingsPanel
    {
    public:
        PhysicsSettingsPanel() = default;
        void Init();
        void OnImGuiRender(bool* show);
    private:
        RendererID mPhysXTextureID;
        Uint mTextureDimensions[2];
    };
}
