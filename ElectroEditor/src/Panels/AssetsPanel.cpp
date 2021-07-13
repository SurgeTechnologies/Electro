//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Core/Input.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "Project/ProjectManager.hpp"

namespace Electro
{
    static EditorModule* sEditorModuleStorage;
    static bool sLoaded = false;

    AssetsPanel::AssetsPanel(void* editorModulePtr)
    {
        sEditorModuleStorage = static_cast<EditorModule*>(editorModulePtr);
        sLoaded = false;
    }

    void AssetsPanel::Init(void* data) {}

    void AssetsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(ASSETS_TITLE, show);




        ImGui::End();
    }
}
