//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PanelManager.hpp"

namespace Electro
{
    PanelMap PanelManager::mPanelMap;

    void PanelManager::PushPanel(const String& name, IPanel* panel, bool* showSwitch, void* initValue)
    {
        mPanelMap[name] = { showSwitch, panel };
        panel->OnInit(initValue);
    }

    void PanelManager::RenderAllPanels()
    {
        for (auto& [name, panel] : mPanelMap)
        {
            bool* showSwitch = panel.Data1;
            IPanel* actualPanel = panel.Data2;
            if(*showSwitch)
                actualPanel->OnImGuiRender(showSwitch);
        }
    }
}