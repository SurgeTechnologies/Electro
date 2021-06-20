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
        void PushPanel(const String& name, IPanel* panel, bool* boolSwitch, void* initValue);
        void RenderAllPanels();

        const IPanel* GetPanel(const String& name) const;
        PanelMap& GetPanelMap() { return mPanelMap; }
    private:
        PanelMap mPanelMap;
    };
}