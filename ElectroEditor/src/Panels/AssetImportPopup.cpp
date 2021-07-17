//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetImportPopup.hpp"
#include "Asset/AssetManager.hpp"
#include "UIUtils/UIUtils.hpp"
#include <imgui.h>

#define TEXTURE_POPUP_KEY "Import Texture2D"
#define ENV_MAP_POPUP_KEY "Import Environment Map"

namespace Electro
{
    static String sPath;
    static bool sLoadDataCheckBox = false;
    static AssetType sType = AssetType::NONE;

    template <typename T, typename... Args>
    static void ImportPopup(const char* key, Args&&... args)
    {
        if (ImGui::BeginPopupModal(key))
        {
            E_ASSERT(!sPath.empty(), "Asset path is empty!");
            E_ASSERT(sType != AssetType::NONE, "Asset type is NONE!");

            if (ImGui::BeginTable("##AssetPopupTable", 2))
            {
                ImGui::TableSetupColumn("##Table", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 60.0f);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Path");
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(AssetManager::GetRelativePath(sPath).c_str());

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Type");
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(Utils::AssetTypeToString(sType));

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Load Data");
                ImGui::TableNextColumn();
                ImGui::Checkbox("##loadDataCheckBox", &sLoadDataCheckBox);
                UI::ToolTip("Instantly Loads the data into RAM for faster access.\nTurn this on if you will use this resource soon!");

                ImGui::EndTable();
            }

            bool clear = false;
            float buttonWidth = ImGui::GetWindowWidth() / 2;

            if (ImGui::Button("Import!", { buttonWidth, 0 }))
            {
                if (!sLoadDataCheckBox)
                    AssetHandle handle = AssetManager::ImportAsset(sPath);
                else
                    AssetManager::CreateNewAsset<T>(sPath, sType, std::forward<Args>(args)...);

                clear = true;
            }

            ImGui::SameLine();

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2);
            if (ImGui::Button("Never Mind", { buttonWidth, 0 }))
                clear = true;

            if (clear)
            {
                sPath.clear();
                sLoadDataCheckBox = false;
                sType = AssetType::NONE;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void AssetImportPopup::CatchImportPopup(const AssetType& type)
    {
        const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (type == AssetType::TEXTURE2D)
            ImportPopup<Texture2D>(TEXTURE_POPUP_KEY, Texture2DSpecification(sPath));
        else if (type == AssetType::ENVIRONMENT_MAP)
            ImportPopup<EnvironmentMap>(ENV_MAP_POPUP_KEY, sPath);
    }

    void AssetImportPopup::ThrowImportPopup(const AssetType& type, const String& path)
    {
        sPath = path;
        sType = type;

        if (type == AssetType::TEXTURE2D)
            ImGui::OpenPopup(TEXTURE_POPUP_KEY);
        if (type == AssetType::ENVIRONMENT_MAP)
            ImGui::OpenPopup(ENV_MAP_POPUP_KEY);
    }
}
