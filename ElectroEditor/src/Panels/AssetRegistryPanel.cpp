//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetRegistryPanel.hpp"
#include "Core/System/OS.hpp"
#include "UIUtils/UIUtils.hpp"
#include "Utility/StringUtils.hpp"
#include <imgui.h>
#include <FontAwesome.hpp>

namespace Electro
{
    void AssetRegistryPanel::OnInit(void* data)
    {
        mRegistrySlot = AssetManager::GetRegistry();
        mLoadedAssetsSlot = AssetManager::GetLoadedAssetsRegistry();
        memset(mAssetRegistrySearchBuffer, 0, INPUT_BUFFER_LENGTH);
        memset(mLoadedAssetSearchBuffer, 0, INPUT_BUFFER_LENGTH);
    }

    void AssetRegistryPanel::RenderAssetInfo(const AssetMetadata& metadata, const char* typeString)
    {
        if (ImGui::Selectable(fmt::format("UUID: {0}", metadata.Handle).c_str()))
            OS::SetClipboardText(fmt::format("{0}", metadata.Handle).c_str());

        if (ImGui::Selectable(fmt::format("Name: {0}", FileSystem::GetNameWithExtension(metadata.Path.string())).c_str()))
            OS::SetClipboardText(AssetManager::GetAbsolutePath(metadata).c_str());

        UI::InstantToolTip(fmt::format("Path: {0}", AssetManager::GetAbsolutePath(metadata)).c_str());

        if (ImGui::Selectable(fmt::format("Type: {0}", typeString).c_str()))
            OS::SetClipboardText(typeString);
    }

    void AssetRegistryPanel::RenderAssetInfoOnSearch(const AssetMetadata& metadata, const char* typeString, bool isNumber)
    {
        if (isNumber) ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.1f, 1.0f });
        if (ImGui::Selectable(fmt::format("UUID: {0}", metadata.Handle).c_str()))
            OS::SetClipboardText(fmt::format("{0}", metadata.Handle).c_str());
        if (isNumber) ImGui::PopStyleColor();

        if (!isNumber) ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.1f, 1.0f });
        if (ImGui::Selectable(fmt::format("Name: {0}", FileSystem::GetNameWithExtension(metadata.Path.string())).c_str()))
            OS::SetClipboardText(AssetManager::GetAbsolutePath(metadata).c_str());
        if (!isNumber) ImGui::PopStyleColor();

        UI::ToolTip(fmt::format("Path: {0}", AssetManager::GetAbsolutePath(metadata)).c_str());

        if (ImGui::Selectable(fmt::format("Type: {0}", typeString).c_str()))
            OS::SetClipboardText(typeString);
    }

    void AssetRegistryPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin("Asset Registry", show);
        if (ImGui::CollapsingHeader("Assets Registry"))
        {
            ImGui::InputTextWithHint("##assetRegSearch", "Type here to search AssetRegistry", mAssetRegistrySearchBuffer, 256);
            ImGui::SameLine();

            if (ImGui::Button("Serialize"))
                AssetManager::SerializeRegistry();

            AssetHandle assetHandleToBeRemoved = INVALID_ASSET_HANDLE;

            for (const auto& [path, metadata] : *mRegistrySlot)
            {
                uint64_t handle = metadata.Handle;
                const char* assetTypeString = Utils::AssetTypeToString(metadata.Type);

                if (mAssetRegistrySearchBuffer[0] == NULL)
                {
                    if (ImGui::BeginTable("##assetsReg", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
                    {
                        ImGui::TableNextColumn();
                        RenderAssetInfo(metadata, assetTypeString);

                        ImGui::TableNextColumn();
                        if (ImGui::Button(ICON_ELECTRO_TRASH))
                            assetHandleToBeRemoved = handle;
                        UI::ToolTip("Removes the asset from registry");

                        ImGui::TextUnformatted("STATUS:");
                        float itemSize = ImGui::CalcTextSize("STATUS:").x + 9.0f;
                        if (AssetManager::IsLoaded(handle))
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, { 0.1f, 0.9f, 0.1f, 1.0f });
                            ImGui::SameLine(itemSize);
                            ImGui::TextUnformatted("LOADED " ICON_ELECTRO_CHECK_SQUARE);
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.8f, 0.1f, 1.0f });
                            ImGui::SameLine(itemSize);
                            ImGui::TextUnformatted("CACHED " ICON_ELECTRO_MINUS_SQUARE);
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndTable();
                    }
                }
                else
                {
                    String searchString = mAssetRegistrySearchBuffer;
                    Utils::ToLower(searchString);

                    if (std::to_string(handle).find(searchString) != std::string::npos || Utils::ToLower(metadata.Path.string()).find(searchString) != std::string::npos)
                    {
                        if (ImGui::BeginTable("##assetsReg", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
                        {
                            ImGui::TableNextColumn();
                            RenderAssetInfoOnSearch(metadata, assetTypeString, Utils::IsNumber(searchString));

                            ImGui::TableNextColumn();
                            if (ImGui::Button(ICON_ELECTRO_TRASH))
                                assetHandleToBeRemoved = handle;
                            UI::ToolTip("Removes the asset from registry");

                            ImGui::TextUnformatted("STATUS:");
                            float itemSize = ImGui::CalcTextSize("STATUS:").x + 9.0f;
                            if (AssetManager::IsLoaded(handle))
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, { 0.1f, 0.9f, 0.1f, 1.0f });
                                ImGui::SameLine(itemSize);
                                ImGui::TextUnformatted("LOADED " ICON_ELECTRO_CHECK_SQUARE);
                                ImGui::PopStyleColor();
                            }
                            else
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.8f, 0.1f, 1.0f });
                                ImGui::SameLine(itemSize);
                                ImGui::TextUnformatted("CACHED " ICON_ELECTRO_MINUS_SQUARE);
                                ImGui::PopStyleColor();
                            }
                            ImGui::EndTable();
                        }
                    }
                }
            }

            if (assetHandleToBeRemoved != INVALID_ASSET_HANDLE)
                AssetManager::RemoveAsset(assetHandleToBeRemoved);
        }

        ImGui::End();
    }
}