//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"
#include "IPanel.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    class AssetsPanel : public IPanel
    {
    public:
        AssetsPanel(void* editorModulePtr);
        ~AssetsPanel() = default;

        virtual void Init(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
    };
}
