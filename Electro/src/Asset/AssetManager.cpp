//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/System/OS.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/EnvironmentMap.hpp"
#include "AssetExtensions.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include <imgui.h>
#include <FontAwesome.hpp>
#include <yaml-cpp/yaml.h>

namespace Electro
{
    void AssetManager::Init()
    {
        AssetLoader::Init();
        DeserializeRegistry();
    }

    void AssetManager::Shutdown()
    {
        SerializeRegistry();
        sLoadedAssets.clear();
        sAssetRegistry.Clear();
        AssetLoader::Shutdown();
    }

    static AssetMetadata sNullMetadata;
    void AssetManager::SerializeRegistry()
    {
        // Sort assets by UUID - then serialize
        struct AssetRegistryEntry
        {
            String FilePath;
            AssetType Type;
        };

        std::map<AssetHandle, AssetRegistryEntry> sortedMap;
        for (auto& [filepath, metadata] : sAssetRegistry)
        {
            String pathToSerialize = metadata.Path.string();
            std::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
            sortedMap[metadata.Handle] = { pathToSerialize, metadata.Type };
        }

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Assets" << YAML::BeginSeq;
        for (auto& [handle, entry] : sortedMap)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
            out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(entry.Type);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        const String& assetRegistryPath = ProjectManager::GetAssetRegistryCachePath().string();
        FileSystem::WriteFile(assetRegistryPath, out.c_str());
    }

    void AssetManager::DeserializeRegistry()
    {
        const String& assetRegistryPath = ProjectManager::GetAssetRegistryCachePath().string();

        if (!FileSystem::Exists(assetRegistryPath))
            return;

        YAML::Node data = YAML::LoadFile(assetRegistryPath);
        YAML::Node handles = data["Assets"];
        if (!handles)
        {
            Log::Error("Invalid AssetRegistry File!");
            return;
        }

        sAssetRegistry.Clear();

        for (const auto& entry : handles)
        {
            String filepath = entry["FilePath"].as<String>();

            AssetMetadata metadata;
            metadata.Handle = entry["Handle"].as<uint64_t>();
            metadata.Path   = filepath; //! filepath is already relative, as we load it from the disk
            metadata.Type   = static_cast<AssetType>(Utils::AssetTypeFromString(entry["Type"].as<String>()));

            if (metadata.Type == AssetType::NONE)
                continue;

            if (!FileSystem::Exists(AssetManager::GetAbsolutePath(metadata)))
            {
                Log::Warn("Missing asset '{0}' found in AssetRegistry file!", metadata.Path.string());
                //TODO: Maybe try to relocate the missing asset here?
                continue;
            }

            if (metadata.Handle == 0)
            {
                Log::Warn("INVALID AssetHandle found for {0}", metadata.Path.string());
                continue;
            }

            sAssetRegistry[metadata.Path] = metadata;
        }
    }

    AssetType AssetManager::GetAssetTypeFromExtension(const String& extension)
    {
        if (sAssetExtensionMap.find(extension) != sAssetExtensionMap.end())
            return sAssetExtensionMap.at(extension);
        else
            return AssetType::NONE;
    }

    String AssetManager::GetAbsolutePath(const AssetMetadata& metadata)
    {
        String result = fmt::format("{0}/{1}", ProjectManager::GetAssetsDirectory().string(), metadata.Path.string());
        return result;
    }

    String AssetManager::GetRelativePath(const String& absolutePath)
    {
        String result = std::filesystem::relative(absolutePath, ProjectManager::GetAssetsDirectory()).string();
        return result;
    }

    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::sLoadedAssets;
    AssetRegistry AssetManager::sAssetRegistry;

    AssetHandle AssetManager::ImportAsset(const String& filepath)
    {
        std::filesystem::path path = std::filesystem::relative(filepath, ProjectManager::GetAssetsDirectory());

        // Already in the registry
        if (sAssetRegistry.Contains(path))
            return INVALID_ASSET_HANDLE;

        AssetType type = GetAssetTypeFromExtension(FileSystem::GetExtension(path.string()));
        if (type == AssetType::NONE)
            return INVALID_ASSET_HANDLE;

        AssetMetadata metadata;
        metadata.Handle = AssetHandle();
        metadata.Path = path; // Relative path is stored
        metadata.Type = type;
        metadata.IsDataLoaded = false;

        // We don't load the data, just import the asset, save to the registry
        sAssetRegistry[metadata.Path] = metadata;
        return metadata.Handle;
    }

    AssetMetadata& AssetManager::GetMetadata(AssetHandle handle)
    {
        for (auto& [filepath, metadata] : sAssetRegistry)
        {
            if (metadata.Handle == handle)
                return metadata;
        }

        return sNullMetadata;
    }

    bool AssetManager::ReloadData(AssetHandle assetHandle)
    {
        auto& metadata = GetMetadata(assetHandle);
        if (!metadata.IsDataLoaded)
        {
            Log::Warn("Trying to Reload Asset that was never loaded");

            Ref<Asset> asset;
            metadata.IsDataLoaded = AssetLoader::TryLoadData(metadata, asset);
            return metadata.IsDataLoaded;
        }

        E_ASSERT(sLoadedAssets.find(assetHandle) != sLoadedAssets.end(), "Asset is not in Loaded asset cache, but its Data is already loaded!");

        Ref<Asset>& asset = sLoadedAssets.at(assetHandle);
        metadata.IsDataLoaded = AssetLoader::TryLoadData(metadata, asset);
        return metadata.IsDataLoaded;
    }

    void AssetManager::RemoveAsset(AssetHandle assetHandle)
    {
        // Check if the given handle exists in the Registry or not
        bool found = false;
        for (const auto& assetRegistryItem : sAssetRegistry)
        {
            if (assetRegistryItem.second.Handle == assetHandle)
            {
                found = true;
                break;
            }
        }
        E_ASSERT(found, "Asset is not present in registry!");

        AssetMetadata metadata = GetMetadata(assetHandle);
        sAssetRegistry.Remove(metadata.Path);

        if (sLoadedAssets.find(assetHandle) != sLoadedAssets.end())
            sLoadedAssets.erase(assetHandle);
    }

    AssetHandle AssetManager::ExistsInRegistry(const String& absPath)
    {
        if (sAssetRegistry.Contains(absPath))
            return sAssetRegistry[absPath].Handle;

        return INVALID_ASSET_HANDLE;
    }

    static void RenderAssetInfo(const AssetMetadata& metadata, const char* typeString)
    {
        if (ImGui::Selectable(fmt::format("UUID: {0}", metadata.Handle).c_str()))
            OS::SetClipboardText(fmt::format("{0}", metadata.Handle).c_str());

        if (ImGui::Selectable(fmt::format("Name: {0}", FileSystem::GetNameWithExtension(metadata.Path.string())).c_str()))
            OS::SetClipboardText(metadata.Path.string().c_str());
        UI::ToolTip(fmt::format("Path: {0}", metadata.Path.string()).c_str());

        if (ImGui::Selectable(fmt::format("Type: {0}", typeString).c_str()))
            OS::SetClipboardText(typeString);

        ImGui::Separator();
    }

    static void RenderAssetInfoOnSearch(const AssetMetadata& metadata, const char* typeString, bool isNumber)
    {
        if (isNumber) ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.1f, 1.0f });
        if (ImGui::Selectable(fmt::format("UUID: {0}", metadata.Handle).c_str()))
            OS::SetClipboardText(fmt::format("{0}", metadata.Handle).c_str());
        if (isNumber) ImGui::PopStyleColor();

        if (!isNumber) ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.1f, 1.0f });
        if (ImGui::Selectable(fmt::format("Name: {0}", FileSystem::GetNameWithExtension(metadata.Path.string())).c_str()))
            OS::SetClipboardText(metadata.Path.string().c_str());
        if (!isNumber) ImGui::PopStyleColor();
        UI::ToolTip(fmt::format("Path: {0}", metadata.Path.string()).c_str());

        if (ImGui::Selectable(fmt::format("Type: {0}", typeString).c_str()))
            OS::SetClipboardText(typeString);

        ImGui::Separator();
    }

    static char sLoadedAssetSearchBuffer[256];
    static char sAssetRegistrySearchBuffer[256];

    void AssetManager::OnImGuiRender(bool* open)
    {
        ImGui::Begin("Asset Manager", open);
        if (ImGui::CollapsingHeader("Loaded Assets"))
        {
            ImGui::InputTextWithHint("##loadedAssetsSearch", "Type here to search LoadedAssets", sLoadedAssetSearchBuffer, 256);

            for (const auto& [handle, asset] : sLoadedAssets)
            {
                AssetHandle assetHandle = asset->GetHandle();

                if (handle == assetHandle)
                {
                    AssetMetadata metadata = GetMetadata(assetHandle);
                    const char* assetTypeString = Utils::AssetTypeToString(metadata.Type);

                    if (sLoadedAssetSearchBuffer[0] == NULL)
                        RenderAssetInfo(metadata, assetTypeString);
                    else
                    {
                        String searchString = sLoadedAssetSearchBuffer;
                        Utils::ToLower(searchString);

                        if (std::to_string(handle).find(searchString) != std::string::npos || Utils::ToLower(metadata.Path.string()).find(searchString) != std::string::npos)
                            RenderAssetInfoOnSearch(metadata, assetTypeString, Utils::IsNumber(searchString));
                    }
                }
                else
                    Log::Error("Invalid asset handle - {0}!", handle);
            }
        }
        if (ImGui::CollapsingHeader("Assets Registry"))
        {
            ImGui::InputTextWithHint("##assetRegSearch", "Type here to search AssetRegistry", sAssetRegistrySearchBuffer, 256);

            ImGui::SameLine();

            if (ImGui::Button("Serialize"))
                SerializeRegistry();

            AssetHandle assetHandleToBeRemoved = 0;
            for (const auto& [path, metadata] : sAssetRegistry)
            {
                uint64_t handle = metadata.Handle;
                const char* assetTypeString = Utils::AssetTypeToString(metadata.Type);

                if (sAssetRegistrySearchBuffer[0] == NULL)
                {
                    if (ImGui::BeginTable("##assetsReg", 2))
                    {
                        ImGui::TableSetupColumn("##AssetsColumn", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 200.0f);
                        ImGui::TableNextColumn();
                        RenderAssetInfo(metadata, assetTypeString);

                        ImGui::TableNextColumn();
                        if (ImGui::Button(ICON_ELECTRO_TRASH))
                            assetHandleToBeRemoved = handle;

                        ImGui::EndTable();
                    }
                }
                else
                {
                    String searchString = sAssetRegistrySearchBuffer;
                    Utils::ToLower(searchString);

                    if (std::to_string(handle).find(searchString) != std::string::npos || Utils::ToLower(metadata.Path.string()).find(searchString) != std::string::npos)
                        RenderAssetInfoOnSearch(metadata, assetTypeString, Utils::IsNumber(searchString));
                }
            }

            if (assetHandleToBeRemoved != 0)
                RemoveAsset(assetHandleToBeRemoved);
        }
        ImGui::End();
    }
}
