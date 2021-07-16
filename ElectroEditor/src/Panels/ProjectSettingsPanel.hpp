//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "IPanel.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    class Project;
    class ProjectSettingsPanel : public IPanel
    {
    public:
        virtual void OnInit(void* data = nullptr);
        virtual void OnImGuiRender(bool* show) override;
    private:
        void SaveSettings();
        void CreateScene(ProjectConfig& projectConfig);
    private:
        Project* mActiveProjectSlot;
        char mInputBuffer[INPUT_BUFFER_LENGTH];
    };
}