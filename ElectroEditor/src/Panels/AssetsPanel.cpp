//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Core/Input.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "Project/ProjectManager.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetExtensions.hpp"

namespace Electro
{
    AssetsPanel::AssetsPanel() {}

    void AssetsPanel::OnInit(void* data)
    {
        mFolderTextureID  = Texture2D::Create({ "Electro/assets/textures/Folder.png" });
        mUnknownTextureID = Texture2D::Create({ "Electro/assets/textures/UnknownIcon.png" });
        m3DFileTextureID = Texture2D::Create({ "Electro/assets/textures/3DFileIcon.png" });
        mImageTextureID = Texture2D::Create({ "Electro/assets/textures/ImageIcon.png" });
    }

    void AssetsPanel::DrawPath(const DirectoryEntry& entry)
    {
        mSkipText = false;
        ImGui::TableNextColumn();

        if (entry.IsDirectory)
        {
            UI::ImageButton(mFolderTextureID->GetRendererID(), { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f));
            if (ImGui::IsItemHovered())
            {
                // Folder is double clicked, so change the drawing directory
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    mCurrentPath = entry.AbsolutePath;
            }
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".hdr")
        {
            UI::ImageButton(mImageTextureID->GetRendererID(), { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f));
            if (ImGui::BeginDragDropSource())
            {
                AssetDropData dropData;
                dropData.Handle = AssetManager::ExistsInRegistry(entry.AbsolutePath);
                dropData.Path = entry.AbsolutePath.c_str();

                ImGui::SetDragDropPayload(TEXTURE_DND_ID, &dropData, sizeof(AssetHandle) + entry.AbsolutePath.size());
                ImGui::EndDragDropSource();
            }
        }
        else
        {
            UI::ImageButton(mUnknownTextureID->GetRendererID(), { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f));
        }

        if (!mSkipText)
            ImGui::TextWrapped(entry.NameWithExtension.c_str());
    }

    const Vector<DirectoryEntry> AssetsPanel::GetFiles(const String& directory)
    {
        Deque<DirectoryEntry> result;
        Vector<DirectoryEntry> vecResult;

        try
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory, std::filesystem::directory_options::skip_permission_denied))
            {
                const std::filesystem::path& path = entry.path();
                DirectoryEntry e;
                e.Name = path.stem().string();
                e.Extension = path.extension().string();
                e.IsDirectory = entry.is_directory();
                e.NameWithExtension = fmt::format("{0}{1}", e.Name, e.Extension);

                e.AbsolutePath = path.string();
                e.ParentFolder = path.parent_path().string();
                std::replace(e.AbsolutePath.begin(), e.AbsolutePath.end(), '\\', '/');
                std::replace(e.ParentFolder.begin(), e.ParentFolder.end(), '\\', '/');

                if (e.IsDirectory)
                    result.push_front(e);
                else
                    result.push_back(e);
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            Log::Error("{0}!", e.what());
        }
        catch (...)
        {
            Log::Error("Encountered error while trying to read files from Disk!");
        }

        for (Uint i = 0; i < result.size(); i++)
            vecResult.emplace_back(std::move(result[i]));

        result.clear();
        return vecResult;
    }

    void AssetsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(ASSETS_TITLE, show);
        ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 0.3f });

        if (ImGui::Button("Refresh"))
        {
            String projectRoot = ProjectManager::GetAssetsDirectory().string();
            mFiles = GetFiles(projectRoot);
            ChangeCurrentPath(projectRoot);
            Log::Info("{0}", mCurrentPath);
        }

        {
            const float itemSize = 65.0f;
            int columns = static_cast<int>(ImGui::GetWindowWidth() / (itemSize + 11.0f));
            columns = columns < 1 ? 1 : columns;
            int index = 0;
            if (ImGui::BeginTable("##AssetsTable", columns, ImGuiTableFlags_SizingFixedSame))
            {
                ImGui::TableSetupColumn("##AssetsColumn", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, itemSize);
                for (const DirectoryEntry& file : mFiles)
                {
                    // Current folder is not equal to the parent folder, so no need to draw that
                    if (file.ParentFolder != mCurrentPath)
                        continue;

                    ImGui::PushID(index++);
                    DrawPath(file);
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }

        ImGui::PopStyleColor();
        ImGui::End();
    }
}
