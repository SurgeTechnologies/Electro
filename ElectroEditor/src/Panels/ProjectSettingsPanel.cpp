//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "Core/FileSystem.hpp"
#include "Scene/Entity.hpp" // Needed for Scene
#include "Scene/Scene.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/ProjectSerializer.hpp"
#include "ProjectSettingsPanel.hpp"
#include "UIUtils/UIUtils.hpp"
#include <fmt/core.h>
#include <imgui.h>
#include <FontAwesome.hpp>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#define PROJECT_SETTINGS_PROPERTY(name, memAdress)\
ImGui::TableNextColumn();                         \
ImGui::TextUnformatted(name);                     \
ImGui::TableNextColumn();                         \
ImGui::SameLine();                                \
ImGui::PushID(name);                              \
ImGui::InputText("", memAdress);                  \
ImGui::PopID();                                   \

namespace Electro
{
    void ProjectSettingsPanel::OnInit(void* data)
    {
        mActiveProjectSlot = ProjectManager::GetActiveProjectSlot();
    }

    void ProjectSettingsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(PROJECT_SETTINGS_TITLE, show);
        mActiveProjectSlot = ProjectManager::GetActiveProjectSlot();
        ProjectConfig& projectConfig = mActiveProjectSlot->GetConfig();

        if (mActiveProjectSlot && !projectConfig.ProjectDirectory.empty())
        {
            if (ImGui::BeginTable("##projSetTable", 1))
            {
                PROJECT_SETTINGS_PROPERTY("Project Name", &projectConfig.ProjectName);
                PROJECT_SETTINGS_PROPERTY("Project Root", &projectConfig.ProjectDirectory);
                PROJECT_SETTINGS_PROPERTY("Assets Directory Name", &projectConfig.AssetsDirectoryName);
                ImGui::EndTable();
            }

            ImGui::Separator();

            UI::TextCentered("Scenes");

            for (Uint i = 0; i < projectConfig.ScenePaths.size(); i++)
            {
                ImGui::Text(fmt::format("Scene {0}", i).c_str());
                ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::InputText("", &projectConfig.ScenePaths[i]);
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.7f, 0.1f, 0.1f, 1.0f });
                if (ImGui::Button("Remove"))
                    projectConfig.ScenePaths.erase(projectConfig.ScenePaths.begin() + i);
                ImGui::PopStyleColor();

                ImGui::PopID();
            }

            const float& width = ImGui::GetWindowWidth();
            if (ImGui::Button("Add Scene", { width, 0 }))
            {
                ImGui::OpenPopup("Add New Scene");
                mInputBuffer = "New Scene";
            }

            const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Add New Scene"))
            {
                String sceneRootPath = (ProjectManager::GetAssetsDirectory() / "Scenes").string();
                ImGui::TextDisabled((fmt::format("Scene file will be created at {0}/{1}", sceneRootPath, mInputBuffer)).c_str());

                ImGui::TextUnformatted("Scene Name");
                ImGui::SameLine();

                // Render the text field (Where user inputs text)
                if (ImGui::InputText("##Scene Name", &mInputBuffer, ImGuiInputTextFlags_EnterReturnsTrue))
                    CreateScene(projectConfig);

                bool noCreateButton = false;
                // Outline the above text box with Red color, indicating, something is wrong!
                if (String(mInputBuffer).empty())
                {
                    UI::DrawRectAroundWidget({ 1.0f, 0.0f, 0.0f, 1.0f }, 2.0f, 1.0f);
                    noCreateButton = true;
                }

                // Get the current popup's width
                const float& popupWidth = ImGui::GetWindowWidth();

                if(!noCreateButton)
                {
                    if (ImGui::Button("Create", { popupWidth / 2, 0 }))
                        CreateScene(projectConfig);

                    ImGui::SameLine();
                }

                if (ImGui::Button("Close", noCreateButton ? ImVec2(popupWidth - 2, 0) : ImVec2(popupWidth / 2, 0)))
                    ImGui::CloseCurrentPopup();

                ImGui::EndPopup();
            }

            if (ImGui::Button("Save and Apply!", { width, 0 }))
                SaveSettings();
        }
        else
            ImGui::Text("Open/Create a project first to show its settings here!");

        ImGui::End();
    }

    void ProjectSettingsPanel::SaveSettings()
    {
        ProjectSerializer projSerializer;
        projSerializer.Serialize(mActiveProjectSlot);
    }

    void ProjectSettingsPanel::CreateScene(ProjectConfig& projectConfig)
    {
        if (!String(mInputBuffer).empty())
        {
            const String sceneName = FileSystem::EnsureExtension((char*)mInputBuffer.c_str(), ".electro");
            const String sceneFilePath = fmt::format("{0}/{1}", (ProjectManager::GetAssetsDirectory() / "Scenes").string(), sceneName);

            Ref<Scene> dummyScene = Ref<Scene>::Create(FileSystem::GetNameWithoutExtension(sceneName));
            SceneSerializer serializer(dummyScene);
            serializer.Serialize(sceneFilePath);
            dummyScene.Release();

            String relativeScenePath = std::filesystem::relative(sceneFilePath, ProjectManager::GetAbsoluteBasePath()).string();
            std::replace(relativeScenePath.begin(), relativeScenePath.end(), '\\', '/');

            projectConfig.ScenePaths.push_back(relativeScenePath);
            ImGui::CloseCurrentPopup();
        }
    }
}
