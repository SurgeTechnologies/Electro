//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetImportPopup.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "UIUtils/UIUtils.hpp"
#include <imgui.h>

namespace Electro
{
    static String sPath;
    static bool sLoadDataCheckBox = false;
    static AssetType sType = AssetType::NONE;

    void AssetImportPopup::CatchTextureImportPopup()
    {
        const ImVec2& center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(TEXTURE_POPUP_KEY))
        {
            E_ASSERT(!sPath.empty(), "Asset path is empty!");
            sType = AssetManager::GetAssetTypeFromExtension(FileSystem::GetExtension(sPath));
            E_ASSERT(sType != AssetType::NONE, "Asset type is NONE!");

            if (ImGui::BeginTable("##AssetTexPopupTable", 2))
            {
                ImGui::TableSetupColumn("##TexTable", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 60.0f);

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

            if (ImGui::Button("Import!"))
            {
                if (!sLoadDataCheckBox)
                    AssetHandle handle = AssetManager::ImportAsset(sPath);
                else
                    AssetManager::CreateNewAsset<Texture2D>(sPath, sType, Texture2DSpecification(sPath));

                clear = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Close"))
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

    void AssetImportPopup::ThrowTextureImportPopup(const String& path)
    {
        sPath = path;
        ImGui::OpenPopup(TEXTURE_POPUP_KEY);
    }
}
