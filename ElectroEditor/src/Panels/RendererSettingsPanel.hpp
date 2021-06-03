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
        glm::vec2 mImageSize = { 200.0f, 200.0f };
        int mIndex = 0;
    };
}