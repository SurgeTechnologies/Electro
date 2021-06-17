//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "Core/Input.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <fstream>
#include "Project/ProjectManager.hpp"

namespace Electro
{
    static EditorModule* sEditorModuleStorage;
    static bool sLoaded = false;

    static String sDefaultScriptText =
R"(using System;
using Electro;

//This function is called once, when the game starts
public void OnStart()
{
    //Your initialization code goes here
}

//This function is called every frame
public void OnUpdate(float ts)
{
    //Your game code goes here
})";

    AssetsPanel::AssetsPanel(void* editorModulePtr)
    {
        sEditorModuleStorage = static_cast<EditorModule*>(editorModulePtr);
        mProjectPath.clear();
        sLoaded = false;
    }

    void AssetsPanel::Init(void* data)
    {
        mFolderTextureID = Texture2D::Create("Electro/assets/textures/Folder.png")->GetRendererID();
        mCSTextureID = Texture2D::Create("Electro/assets/textures/CSharpIcon.png")->GetRendererID();
        mElectroTextureID = Texture2D::Create("Electro/assets/textures/ElectroIcon.png")->GetRendererID();
        mUnknownTextureID = Texture2D::Create("Electro/assets/textures/UnknownIcon.png")->GetRendererID();
        m3DFileTextureID = Texture2D::Create("Electro/assets/textures/3DFileIcon.png")->GetRendererID();
        mImageTextureID = Texture2D::Create("Electro/assets/textures/ImageIcon.png")->GetRendererID();
        mMaterialTextureID = Texture2D::Create("Electro/assets/textures/Material.png")->GetRendererID();
        mPhysicsMatTextureID = Texture2D::Create("Electro/assets/textures/PhysicsMaterial.png")->GetRendererID();

        mRenaming = false;
        memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
    }

    void AssetsPanel::OnImGuiRender(bool* show)
    {
        ImGui::Begin(ASSETS_TITLE, show);
        mAssetsPanelFocused = ImGui::IsWindowFocused();

        if (ImGui::Button("Refresh"))
        {
            mProjectPath = ProjectManager::GetAssetsDirectory().string();
            mFiles = FileSystem::GetFiles(mProjectPath);
            UpdateSplitStringBuffer();
        }
        ImGui::SameLine();

        if (ImGui::Button("Root"))
        {
            mDrawingPath = mProjectPath;
        }
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
                    memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
                    if (ImGui::InputText("FolderName", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        FileSystem::CreateOrEnsureFolderExists(mDrawingPath, mRenameBuffer);
                        mFiles = FileSystem::GetFiles(mProjectPath);
                        mSkipText = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            {
                if (ImGui::Button("Scene"))
                    ImGui::OpenPopup("ScenePopup");
                if (ImGui::BeginPopup("ScenePopup"))
                {
                    memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
                    if (ImGui::InputText("SceneName", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        //Ensure that there is a '.electro' extension
                        String projectName = EnsureExtension(".electro");

                        String path = mDrawingPath + "/" + projectName;
                        std::ofstream stream = std::ofstream(path);
                        if (stream.bad())
                            ELECTRO_ERROR("Bad stream!");

                        sEditorModuleStorage->InitSceneEssentials();
                        SceneSerializer serializer(sEditorModuleStorage->mEditorScene, sEditorModuleStorage);
                        serializer.Serialize(path);
                        sEditorModuleStorage->UpdateWindowTitle(projectName);
                        sEditorModuleStorage->mActiveFilepath = path;
                        mFiles = FileSystem::GetFiles(mProjectPath);

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
                    memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
                    if (ImGui::InputText("Material Name", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        //Ensure that there is a '.emat' extension with the name
                        String matName = EnsureExtension(".emat");
                        String path = mDrawingPath + "/" + matName;
                        std::ofstream out(path);

                        //Temporary material
                        Ref<Material> temp = Ref<Material>::Create(Renderer::GetShader("PBR"), "Material", path);
                        temp->Set<glm::vec3>("Material.Albedo", { 1.0f, 1.0f, 1.0f });
                        temp->Set<float>("Material.AO", 1.0f);
                        temp->Serialize();
                        temp.Reset();

                        mFiles = FileSystem::GetFiles(mProjectPath);
                        mSkipText = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            {
                if (ImGui::Button("Physics Material"))
                    ImGui::OpenPopup("PhysicsMaterialPopup");
                if (ImGui::BeginPopup("PhysicsMaterialPopup"))
                {
                    memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
                    if (ImGui::InputText("PhysicsMaterial Name", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        //Ensure that there is a '.epmat' extension with the name
                        String matName = EnsureExtension(".epmat");

                        String path = mDrawingPath + "/" + matName;
                        Ref<PhysicsMaterial> asset = PhysicsMaterial::Create(path);
                        asset->Serialize();

                        mFiles = FileSystem::GetFiles(mProjectPath);
                        mSkipText = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            {
                if (ImGui::Button("Script"))
                    ImGui::OpenPopup("ScriptPopup");
                if (ImGui::BeginPopup("ScriptPopup"))
                {
                    memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
                    if (ImGui::InputText("ScriptName", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        //Ensure that there is a '.cs' extension
                        String scriptName = EnsureExtension(".cs");

                        //Create the file
                        FileSystem::WriteFile(mDrawingPath + "/" + scriptName, sDefaultScriptText);
                        mFiles = FileSystem::GetFiles(mProjectPath);
                        //TODO:
                        // Register to All-Script's Buffer
                        // Drag and drop to script component

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
                    mDrawingPath = mProjectPath;
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

                ImGui::SameLine(0, 1.0f); //This handles the spacing between the buttons
                ImGui::TextColored(UI::GetStandardColorImVec4(), "/");
                ImGui::SameLine(0, 1.0f); // ^^
            }

            ImGui::TextUnformatted(""); //Yes this is needed
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
                for (auto& file : mFiles)
                {
                    //Draw folder is not equal to the parent folder, so no need to draw that
                    if (file.ParentFolder != mDrawingPath)
                        continue;

                    ImGui::PushID(index++);
                    DrawPath(file);
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }

    void AssetsPanel::Load()
    {
        mProjectPath = ProjectManager::GetAssetsDirectory().string();
        mDrawingPath = mProjectPath;
        mFiles = FileSystem::GetFiles(mProjectPath);
        UpdateSplitStringBuffer();
    }

    void AssetsPanel::DrawPath(DirectoryEntry& entry)
    {
        // Set the skip text to false initially
        mSkipText = false;

        ImGui::TableNextColumn();

        // Render the folder
        if (entry.IsDirectory)
        {
            UI::ImageButton(mFolderTextureID, { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f));
            if (ImGui::IsItemHovered())
            {
                //Folder is double clicked, so change the drawing directory
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    mDrawingPath = entry.AbsolutePath;
                    UpdateSplitStringBuffer();
                }

                //Folder is clicked once, so set it as the selected entry
                if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
                    mSelectedEntry = entry;
            }

            if (mSelectedEntry == entry)
            {
                UI::DrawRectAroundWidget(UI::GetStandardColorGLMVec4(), 3.0f, 3.5f);
                HandleRenaming();
                HandleDeleting(entry);
            }
        }

        // Other file types
        else if (entry.Extension == ".electro")
        {
            HandleExtension(entry, mElectroTextureID);
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop at Viewport to open this scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            HandleExtension(entry, mImageTextureID);
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop where Texture is needed");
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds" || entry.Extension == ".FBX")
        {
            HandleExtension(entry, m3DFileTextureID);
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop where Mesh is needed");
        }
        else if (entry.Extension == ".cs")
        {
            HandleExtension(entry, mCSTextureID);
        }
        else if (entry.Extension == ".emat")
        {
            HandleExtension(entry, mMaterialTextureID);
            UI::DragAndDropSource(MATERIAL_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop where material is needed");
        }
        else if (entry.Extension == ".epmat")
        {
            HandleExtension(entry, mPhysicsMatTextureID);
            UI::DragAndDropSource(PHYSICS_MAT_DND_ID, &entry.AbsolutePath, static_cast<int>(entry.AbsolutePath.size()), "Drop in RigidbodyComponent to set this material");
        }
        else
            HandleExtension(entry, mUnknownTextureID);

        if (!mSkipText)
            ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
    }

    void AssetsPanel::UpdateSplitStringBuffer()
    {
        mSplitBuffer.clear();
        mSplitBuffer = Utils::SplitString(mDrawingPath, "/\\");
    }

    void AssetsPanel::HandleExtension(DirectoryEntry& entry, const RendererID texID)
    {
        if (UI::ImageButton(texID, { 50.0f, 50.0f }, ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f)))
        {
            mSelectedEntry = entry;
            UpdateSplitStringBuffer();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            mSelectedEntry = entry;

        // This function is called in a for loop, we need to make sure that the selected asset only recives the
        // renaming, deleting and other stuff
        if (mSelectedEntry == entry)
        {
            UI::DrawRectAroundWidget(UI::GetStandardColorGLMVec4(), 3.0f, 3.5f);
            HandleRenaming();
            HandleDeleting(entry);
        }
    }

    void AssetsPanel::StartRenaming()
    {
        memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
        memcpy(mRenameBuffer, mSelectedEntry.Name.c_str(), mSelectedEntry.Name.size());
        mRenaming = true;
        ImGui::SetKeyboardFocusHere();
    }

    void AssetsPanel::HandleRenaming()
    {
        // If Key::F2 is pressed it starts Renaming the file
        if (!mRenaming && Input::IsKeyPressed(Key::F2) && mAssetsPanelFocused)
            StartRenaming();

        // If Key::Escape is pressed it stops Renaming the file
        if (mRenaming && Input::IsKeyPressed(Key::Escape) && mAssetsPanelFocused)
        {
            mRenaming = false;
            mSkipText = false;
            memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
        }

        // Renaming is ongoing
        if (mRenaming)
        {
            if (ImGui::InputText("##rename", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                // If user pressed Key::Enter, this scope is executed
                FileSystem::RenameFile(mSelectedEntry.AbsolutePath, mRenameBuffer);
                mFiles = FileSystem::GetFiles(mProjectPath);
                mRenaming = false;
                mSkipText = true;
                memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
            }
        }
    }

    void AssetsPanel::HandleDeleting(DirectoryEntry& entry)
    {
        if (!mRenaming && Input::IsKeyPressed(Key::Delete) && mAssetsPanelFocused)
            ImGui::OpenPopup(ICON_ELECTRO_EXCLAMATION_TRIANGLE" Delete File");

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2{ 400,0 });

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
                    AssetManager::Remove(mSelectedEntry.AbsolutePath);
                    FileSystem::Deletefile(mSelectedEntry.AbsolutePath);
                }
                else
                {
                    const Vector<DirectoryEntry> files = FileSystem::GetFiles(mSelectedEntry.AbsolutePath, FileFetchType::ExcludingFolder);
                    // Delete all sub-folders, files in this ^ folder
                    for (const DirectoryEntry& subEntry : files)
                    {
                        AssetManager::Remove(subEntry.AbsolutePath);
                        FileSystem::Deletefile(subEntry.AbsolutePath);
                    }
                    FileSystem::Deletefile(mSelectedEntry.AbsolutePath); //Delete the folder
                }

                mSkipText = true;
                mFiles = FileSystem::GetFiles(mProjectPath);
            }
            ImGui::SameLine();
            if (ImGui::Button("No"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    String AssetsPanel::SearchAssets(const String& query)
    {
        for (const DirectoryEntry& file : mFiles)
        {
            // We don't allow searching directories
            if(file.IsDirectory)
                continue;

            if ((file.Name + file.Extension).find(query) != std::string::npos)
                return file.ParentFolder;
        }
        return mProjectPath;
    }

    String AssetsPanel::EnsureExtension(const String& ext) const
    {
        String name = String(mRenameBuffer);
        const String extension = FileSystem::GetExtension(name);
        extension.empty() ? name.append(ext) : name;
        return name;
    }
}
