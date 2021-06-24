//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PanelManager.hpp"

namespace Electro
{
    void PanelManager::PushPanel(const String& name, IPanel* panel, bool* showSwitch, void* initValue)
    {
        mPanelMap[name] = { showSwitch, panel };
        panel->Init(initValue);
    }

    const IPanel* PanelManager::GetPanel(const String& name) const
    {
        for (auto& [panelName, panel] : mPanelMap)
        {
            if (panelName == name)
                return panel.Data2;
        }
        Log::Info("No Panel found with name {0}!", name);
        return nullptr;
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