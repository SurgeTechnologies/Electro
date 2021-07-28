//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Panels/IPanel.hpp"

namespace Electro
{
    // Mapped as-> PanelName --- (ShowSwitch - Panel)
    using PanelMap = std::unordered_map<String, Pair<bool*, IPanel*>>;

    class PanelManager
    {
    public:
        static void PushPanel(const String& name, IPanel* panel, bool* boolSwitch, void* initValue);
        static void RenderAllPanels();

        template <typename T>
        static T* GetPanel(const String& name)
        {
            for (auto& [panelName, panel] : mPanelMap)
            {
                if (panelName == name)
                    return static_cast<T*>(panel.Data2);
            }
            Log::Info("No Panel found with name {0}!", name);
            return nullptr;
        }

        static PanelMap& GetPanelMap() { return mPanelMap; }
    private:
        static PanelMap mPanelMap;
    };
}