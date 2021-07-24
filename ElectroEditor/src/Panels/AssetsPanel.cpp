//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Core/Input.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "Project/ProjectManager.hpp"

#define ZERO_BUFFER(x) std::memset(x, 0, INPUT_BUFFER_LENGTH)

namespace Electro
{
    static EditorModule* sEditorModuleStorage;

    AssetsPanel::AssetsPanel(void* editorModulePtr)
    {
        sEditorModuleStorage = static_cast<EditorModule*>(editorModulePtr);
    }

    void AssetsPanel::OnInit(void* data)
    {
        mFolderTex = Texture2D::Create({ "Electro/assets/textures/Folder.png" });
        mElectroTex = Texture2D::Create({ "Electro/assets/textures/Electro.png" });
        mUnknownTex = Texture2D::Create({ "Electro/assets/textures/UnknownIcon.png" });
        m3DFileTex = Texture2D::Create({ "Electro/assets/textures/3DFileIcon.png" });
        mImageTex = Texture2D::Create({ "Electro/assets/textures/ImageIcon.png" });
        mMaterialTex = Texture2D::Create({ "Electro/assets/textures/Material.png" });
        mPhysicsMatTex = Texture2D::Create({ "Electro/assets/textures/PhysicsMaterial.png" });
        ZERO_BUFFER(mNameBuffer);
    }

    void AssetsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(ASSETS_TITLE, show);
        ImGui::PushStyleColor(ImGuiCol_Button, { 0.3f, 0.3f, 0.3f, 0.5f });

        mAssetsPanelFocused = ImGui::IsWindowFocused();

        if (ImGui::Button("Root"))
            Load();

        ImGui::SameLine();

        if (ImGui::Button("Create"))
            ImGui::OpenPopup("CreatePopup");

        // Create Popups
        if (ImGui::BeginPopup("CreatePopup"))
        {
            {
                if (ImGui::Button("Folder"))
                    ImGui::OpenPopup("FolderPopup");

                if (ImGui::BeginPopup("FolderPopup"))
                {
                    ZERO_BUFFER(mNameBuffer);
                    if (ImGui::InputText("FolderName", mNameBuffer, sizeof(mNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        FileSystem::CreateOrEnsureFolderExists(mDrawingPath, mNameBuffer);
                        mFiles = GetFiles(ProjectManager::GetAssetsDirectory().string());
                        mSkipText = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            {
                if (ImGui::Button("Material"))
                    ImGui::OpenPopup("MaterialPopup");

                if (ImGui::BeginPopup("MaterialPopup"))
                {
                    if (ImGui::InputText("MaterialName", mNameBuffer, sizeof(mNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        String materialFilepath = fmt::format("{0}/{1}", mDrawingPath, mNameBuffer);
                        Ref<Material> mat = AssetManager::CreateNewAsset<Material>(materialFilepath, AssetType::MATERIAL, Renderer::GetShader("PBR"), "Material", mNameBuffer);
                        AssetLoader::Serialize(AssetManager::GetMetadata(mat->GetHandle()), mat.As<Asset>());

                        mFiles = GetFiles(ProjectManager::GetAssetsDirectory().string());
                        mSkipText = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();

        //Search box
        {
            ImGui::PushItemWidth(200);
            if (UI::TextWithHint(ICON_ELECTRO_SEARCH, &mSearchBuffer, "Search Assets"))
            {
                if (mSearchBuffer.empty())
                {
                    mDrawingPath = ProjectManager::GetAssetsDirectory().string();
                    UpdateSplitStringBuffer();
                }
                else
                {
                    mDrawingPath = SearchAssets(mSearchBuffer);
                    UpdateSplitStringBuffer();
                }
            }
            ImGui::PopItemWidth();
        }

        {
            for (const String& str : mTempSplitBuffer)
            {
                if (ImGui::Button(str.c_str()))
                {
                    mDrawingPath = mDrawingPath.substr(0, mDrawingPath.find(str) + str.size());
                    UpdateSplitStringBuffer();
                }

                ImGui::SameLine(0, 1.0f); // This handles the spacing between the buttons
                ImGui::TextColored(UI::GetStandardColorImVec4(), " " ICON_ELECTRO_CARET_RIGHT " ");
                ImGui::SameLine(0, 1.0f); // ^^
            }

            ImGui::TextUnformatted(""); // Yes this is needed
            mTempSplitBuffer = mSplitBuffer;
        }

        {
            const float itemSize = 65.0f;
            int columns = static_cast<int>(ImGui::GetWindowWidth() / (itemSize + 11.0f));
            columns = columns < 1 ? 1 : columns;
            int index = 0;
            if (ImGui::BeginTable("##AssetsTable", columns, ImGuiTableFlags_SizingFixedSame))
            {
                ImGui::TableSetupColumn("##AssetsColumn", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, itemSize);
                for (const auto& file : mFiles)
                {
                    // Draw folder is not equal to the parent folder, so no need to draw that
                    if (file.ParentFolder != mDrawingPath)
                        continue;

                    ImGui::PushID(index++);
                    bool pop = false;
                    if (mSelectedEntry.AbsolutePath == file.AbsolutePath)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                        pop = true;
                    }
                    DrawPath(file);
                    if (pop)
                        ImGui::PopStyleColor();
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }

        ImGui::PopStyleColor();
        ImGui::End();
    }

    void AssetsPanel::Load()
    {
        String projectPath = ProjectManager::GetAssetsDirectory().string();
        mDrawingPath = projectPath;
        mFiles = GetFiles(projectPath);
        UpdateSplitStringBuffer();
    }

    void AssetsPanel::DrawPath(const DirectoryEntry& entry)
    {
        // Set the skip text to false initially
        mSkipText = false;

        ImGui::TableNextColumn();

        // Render the folder
        if (entry.IsDirectory)
        {
            UI::ImageButton(mFolderTex->GetRendererID(), { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f));
            if (ImGui::IsItemHovered())
            {
                // Folder is double clicked, so change the drawing directory
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    mDrawingPath = entry.AbsolutePath;
                    UpdateSplitStringBuffer();
                }

                // Folder is clicked once, so set it as the selected entry
                if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
                    mSelectedEntry = entry;
            }

            if (mSelectedEntry.AbsolutePath == entry.AbsolutePath)
                HandleDeleting(entry);
        }

        // Other file types
        else if (entry.Extension == ".electro")
        {
            HandleExtension(entry, mElectroTex->GetRendererID());
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop at Viewport to open this scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            HandleExtension(entry, mImageTex->GetRendererID());
            if (ImGui::BeginDragDropSource())
            {
                AssetDropData dropData;
                dropData.Handle = AssetManager::GetHandleFromPath(entry.AbsolutePath);
                dropData.Path = entry.AbsolutePath.c_str();

                ImGui::SetDragDropPayload(TEXTURE_DND_ID, &dropData, static_cast<int>(sizeof(AssetHandle) + entry.AbsolutePath.size()));
                ImGui::TextUnformatted("Texture");
                ImGui::EndDragDropSource();
            }
        }
        else if (entry.Extension == ".emat")
        {
            HandleExtension(entry, mMaterialTex->GetRendererID());
            if (ImGui::BeginDragDropSource())
            {
                AssetHandle handle = AssetManager::GetHandleFromPath(entry.AbsolutePath);

                ImGui::SetDragDropPayload(MATERIAL_DND_ID, &handle, static_cast<int>(sizeof(AssetHandle)));
                ImGui::TextUnformatted("Material");
                ImGui::EndDragDropSource();
            }
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds" || entry.Extension == ".FBX")
        {
            HandleExtension(entry, m3DFileTex->GetRendererID());
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop where Mesh is needed");
        }
        else if (entry.Extension == ".epmat")
        {
            HandleExtension(entry, mPhysicsMatTex->GetRendererID());
            UI::DragAndDropSource(PHYSICS_MAT_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop in RigidbodyComponent to set this material");
        }
        else
            HandleExtension(entry, mUnknownTex->GetRendererID());

        if (!mSkipText)
            ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
    }

    void AssetsPanel::UpdateSplitStringBuffer()
    {
        mSplitBuffer.clear();
        mSplitBuffer = Utils::SplitString(mDrawingPath, "/\\");
    }

    // This function is called in a for loop
    void AssetsPanel::HandleExtension(const DirectoryEntry& entry, const RendererID texID)
    {
        if (UI::ImageButton(texID, { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f)))
        {
            mSelectedEntry = entry;
            UpdateSplitStringBuffer();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            mSelectedEntry = entry;

        if (mSelectedEntry.AbsolutePath == entry.AbsolutePath)
            HandleDeleting(entry);
    }

    void AssetsPanel::HandleDeleting(const DirectoryEntry& entry)
    {
        if (Input::IsKeyPressed(Key::Delete) && mAssetsPanelFocused)
            ImGui::OpenPopup(ICON_ELECTRO_EXCLAMATION_TRIANGLE" Delete File");

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 0));

        if (ImGui::BeginPopupModal(ICON_ELECTRO_EXCLAMATION_TRIANGLE" Delete File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("Are you sure that you want to delete -");
            ImGui::SameLine();
            if (!entry.IsDirectory)
            {
                ImGui::TextColored(UI::GetStandardColorImVec4(), "%s?", (mSelectedEntry.Name + mSelectedEntry.Extension).c_str());
                ImGui::TextColored({ 1.0f, 0.9f, 0.0f, 1.0f }, "Once deleted, you cannot recover this file!");
            }
            else
            {
                ImGui::TextColored(UI::GetStandardColorImVec4(), "%s and all of its contents?", (mSelectedEntry.Name).c_str());
                ImGui::TextColored({ 1.0f, 0.9f, 0.0f, 1.0f }, "Once deleted, you cannot recover this folder!");
            }

            ImGui::SetCursorPosX(static_cast<float>(ImGui::GetWindowWidth() / 2.5));
            if (ImGui::Button("Yes"))
            {
                // Make sure it is removed from asset manager - we don't want any dangling resources which eat memory
                if (!entry.IsDirectory)
                {
                    AssetManager::RemoveAsset(AssetManager::GetHandleFromPath(mSelectedEntry.AbsolutePath));
                    FileSystem::Deletefile(mSelectedEntry.AbsolutePath);
                }
                else
                {
                    const Vector<DirectoryEntry> files = GetFiles(mSelectedEntry.AbsolutePath);
                    // Delete all sub-folders, files in this ^ folder
                    for (const DirectoryEntry& subEntry : files)
                    {
                        AssetManager::RemoveAsset(AssetManager::GetHandleFromPath(subEntry.AbsolutePath));
                        FileSystem::Deletefile(subEntry.AbsolutePath);
                    }
                    FileSystem::Deletefile(mSelectedEntry.AbsolutePath); //Delete the folder
                }

                mSkipText = true;
                mFiles = GetFiles(ProjectManager::GetAssetsDirectory().string());
            }
            ImGui::SameLine();
            if (ImGui::Button("Never Mind"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    String AssetsPanel::SearchAssets(const String& query)
    {
        for (const DirectoryEntry& file : mFiles)
        {
            // We don't allow searching directories
            if (file.IsDirectory)
                continue;

            if ((file.Name + file.Extension).find(query) != std::string::npos)
                return file.ParentFolder;
        }
        return ProjectManager::GetAssetsDirectory().string();
    }

    Vector<DirectoryEntry> AssetsPanel::GetFiles(const String& directory)
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

                //std::replace(e.AbsolutePath.begin(), e.AbsolutePath.end(), '\\', '/');
                //std::replace(e.ParentFolder.begin(), e.ParentFolder.end(), '\\', '/');

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
}
