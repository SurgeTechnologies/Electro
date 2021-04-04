//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once

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