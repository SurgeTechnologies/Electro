//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include <glm/glm.hpp>

namespace Electro
{
    class RendererSettingsPanel
    {
    public:
        void OnImGuiRender(bool show);
    private:
        int mCascadeIndex = 0;
    };
}