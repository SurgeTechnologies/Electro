//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    struct RendererData;
    class RendererSettingsPanel : public IPanel
    {
    public:
        RendererSettingsPanel() = default;
        ~RendererSettingsPanel() = default;

        virtual void OnInit(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
    private:
        RendererData* mRendererData;
        int mCascadeIndex = 0;
    };
}