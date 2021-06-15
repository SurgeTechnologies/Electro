//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Panels/ConsolePanel.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ProfilerPanel.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/MaterialPanel.hpp"
#include "Panels/PhysicsSettingsPanel.hpp"
#include "Panels/RendererSettingsPanel.hpp"

namespace Electro
{
    // Mapped as-> PanelName --- (ShowSwitch - Panel)
    using PanelMap = std::unordered_map<String, Pair<bool*, IPanel*>>;

    class PanelManager
    {
    public:
        void PushPanel(const String& name, IPanel* panel, bool* boolSwitch, void* initValue);
        IPanel* GetPanel(const String& name);
        void RenderAllPanels();
    private:
        PanelMap mPanelMap;
    };
}