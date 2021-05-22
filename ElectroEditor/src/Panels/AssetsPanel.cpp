//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "Asset/AssetSerializer.hpp"
#include "Core/System/OS.hpp"
#include "Core/Input.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <fstream>

namespace Electro
{
    static EditorModule* sEditorModuleStorage;
    static Ref<Texture2D> sTexturePreviewStorage;
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

    AssetsPanel::AssetsPanel(const void* editorModulePtr)
    {
        sEditorModuleStorage = (EditorModule*)editorModulePtr;
        mProjectPath.clear();
        sLoaded = false;
    }

    void AssetsPanel::Init()
    {
        mFolderTextureID     = AssetManager::Get<Texture2D>("Folder.png")->GetRendererID();
        mCSTextureID         = AssetManager::Get<Texture2D>("CSharpIcon.png")->GetRendererID();
        mElectroTextureID    = AssetManager::Get<Texture2D>("ElectroIcon.png")->GetRendererID();
        mUnknownTextureID    = AssetManager::Get<Texture2D>("UnknownIcon.png")->GetRendererID();
        m3DFileTextureID     = AssetManager::Get<Texture2D>("3DFileIcon.png")->GetRendererID();
        mImageTextureID      = AssetManager::Get<Texture2D>("ImageIcon.png")->GetRendererID();
        mMaterialTextureID   = AssetManager::Get<Texture2D>("Material.png")->GetRendererID();
        mPhysicsMatTextureID = AssetManager::Get<Texture2D>("PhysicsMaterial.png")->GetRendererID();
        sTexturePreviewStorage = AssetManager::Get<Texture2D>("Prototype.png");
        mRenaming = false;
        memset(mRenameBuffer, 0, 128);
    }

    void AssetsPanel::OnImGuiRender(bool* show)
    {
        if (mProjectPath.empty())
        {
            mProjectPath = AssetManager::GetProjectPath();
            mDrawingPath = mProjectPath;
            UpdateSplitStringBuffer();
        }

        if (!sLoaded && !mProjectPath.empty())
        {
            mFiles = FileSystem::GetFiles(mProjectPath);
            sLoaded = true;
        }

        ImGui::Begin(ASSETS_TITLE, show);
        mAssetsPanelFocused = ImGui::IsWindowFocused();
        if (AssetManager::IsInitialized() && ImGui::Button("Refresh"))
        {
            mProjectPath = AssetManager::GetProjectPath();
            mFiles = FileSystem::GetFiles(mProjectPath);
        }
        UI::ToolTip("Press this when you add new files");
        ImGui::SameLine();

        if (ImGui::Button("Root"))
        {
            mDrawingPath = mProjectPath;
            UpdateSplitStringBuffer();
        }
        UI::ToolTip("Go to Project Root directory");
        ImGui::SameLine();

        if (ImGui::Button("Create"))
            ImGui::OpenPopup("CreatePopup");

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
                        Ref<Material> asset = Factory::CreateMaterial(AssetManager::Get<Shader>("PBR.hlsl"), "Material", path);
                        AssetSerializer::SerializeMaterial(path, asset);

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
                        Ref<PhysicsMaterial> asset = Factory::CreatePhysicsMaterial(path);
                        AssetSerializer::SerializePhysicsMaterial(path, asset);

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

        UI::ToolTip("Create assets!");
        ImGui::SameLine();

        {   //Search box
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

        if (AssetManager::IsInitialized())
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

        RenderTexturePreviewPanel();
    }

    void AssetsPanel::DrawPath(DirectoryEntry& entry)
    {
        mSkipText = false;
        ImGui::TableNextColumn();

        //Render the folder
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
                HandleRenaming(entry);
                HandleDeleting(entry);
            }

            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str()); return;
        }

        //Other file types
        else if (entry.Extension == ".electro")
        {
            HandleExtension(entry, mElectroTextureID);
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop at Viewport to open this scene");
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            HandleExtension(entry, mImageTextureID);
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop where Texture is needed");
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds" || entry.Extension == ".FBX")
        {
            HandleExtension(entry, m3DFileTextureID);
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop where Mesh is needed");
            if(!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else if (entry.Extension == ".cs")
        {
            HandleExtension(entry, mCSTextureID);
            UI::DragAndDropSource(READABLE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop in " CODE_EDITOR_TITLE " to open this file");
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else if (entry.Extension == ".emat")
        {
            HandleExtension(entry, mMaterialTextureID);
            UI::DragAndDropSource(MATERIAL_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop where material is needed");
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else if (entry.Extension == ".epmat")
        {
            HandleExtension(entry, mPhysicsMatTextureID);
            UI::DragAndDropSource(PHYSICS_MAT_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop in RigidbodyComponent to set this material");
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
        else
        {
            HandleExtension(entry, mUnknownTextureID);
            if (!mSkipText)
                ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
            return;
        }
    }

    void AssetsPanel::UpdateSplitStringBuffer()
    {
        mSplitBuffer.clear();
        mSplitBuffer = Utils::SplitString(mDrawingPath, "/\\");
    }

    void AssetsPanel::HandleExtension(DirectoryEntry& entry, RendererID texID)
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
            HandleRenaming(entry);
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

    void AssetsPanel::HandleRenaming(DirectoryEntry& entry)
    {
        //If Key::F2 is pressed it starts Renaming the file
        if (!mRenaming && Input::IsKeyPressed(Key::F2) && mAssetsPanelFocused)
            StartRenaming();

        //If Key::Escape is pressed it stops Renaming the file
        if (mRenaming && Input::IsKeyPressed(Key::Escape) && mAssetsPanelFocused)
        {
            mRenaming = false;
            mSkipText = false;
            memset(mRenameBuffer, 0, INPUT_BUFFER_LENGTH);
        }

        //Renaming is ongoing
        if (mRenaming)
        {
            if (ImGui::InputText("##kekw_rename", mRenameBuffer, sizeof(mRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                //If user pressed Key::Enter, this scope is executed
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

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2{ 400,0 });
        if (ImGui::BeginPopupModal(ICON_ELECTRO_EXCLAMATION_TRIANGLE" Delete File", false, ImGuiWindowFlags_AlwaysAutoResize))
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

            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.5);
            if (ImGui::Button("Yes"))
            {
                //Make sure it is removed from asset manager - we don't want any dangling resources which eat memory
                if (!entry.IsDirectory)
                {
                    AssetManager::RemoveIfExists(mSelectedEntry.AbsolutePath);
                    FileSystem::Deletefile(mSelectedEntry.AbsolutePath);
                }
                else
                {
                    const Vector<DirectoryEntry> files = FileSystem::GetFiles(mSelectedEntry.AbsolutePath, FileFetchType::ExcludingFolder);
                    //Delete all sub-folders, files in this ^ folder
                    for (const DirectoryEntry& entry : files)
                    {
                        AssetManager::RemoveIfExists(entry.AbsolutePath);
                        FileSystem::Deletefile(entry.AbsolutePath);
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

    const String AssetsPanel::SearchAssets(const String& query)
    {
        for (DirectoryEntry& file : mFiles)
        {
            //We don't allow searching directories
            if(file.IsDirectory)
                continue;

            if ((file.Name + file.Extension).find(query) != std::string::npos)
                return file.ParentFolder;
        }
        return mProjectPath;
    }

    String AssetsPanel::EnsureExtension(const String& ext)
    {
        String name = String(mRenameBuffer);
        String extension = FileSystem::GetExtension(name);
        extension == "" ? name.append(ext) : name;
        return name;
    }

    void AssetsPanel::DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes)
    {
        glm::vec2 imageMiddle = { imageRes.x * 0.5f, imageRes.y * 0.5f };
        glm::vec2 windowMiddle = { windowRes.x * 0.5f, windowRes.y * 0.5f };
        glm::vec2 result = { windowMiddle - imageMiddle };
        ImGui::SetCursorPos({ result.x, result.y });
    }

    void AssetsPanel::RenderTexturePreviewPanel()
    {
        ImGui::Begin(TEXTURE_PREVIEW_TITLE, false, ImGuiWindowFlags_HorizontalScrollbar);
        if (sTexturePreviewStorage)
        {
            auto rendererID = sTexturePreviewStorage->GetRendererID();
            glm::vec2 imageRes = { sTexturePreviewStorage->GetWidth(), sTexturePreviewStorage->GetHeight() };
            ImVec2 windowRes = ImGui::GetWindowSize();

            DrawImageAtMiddle(imageRes, { windowRes.x, windowRes.y });
            UI::Image(rendererID, { imageRes.x, imageRes.y });
        }
        auto data = UI::DragAndDropTarget(TEXTURE_DND_ID);
        if (data)
        {
            sTexturePreviewStorage.Reset();
            sTexturePreviewStorage = Factory::CreateTexture2D(*(String*)data->Data);
            glm::vec2 imageRes = { sTexturePreviewStorage->GetWidth(), sTexturePreviewStorage->GetHeight() };
            ImVec2 windowRes = ImGui::GetWindowSize();
            DrawImageAtMiddle(imageRes, { windowRes.x, windowRes.y });
            UI::Image(sTexturePreviewStorage->GetRendererID(), { imageRes.x, imageRes.y });
        }
        ImGui::End();
    }

    Ref<Texture2D>& Electro::GetTexturePreviewtorage()
    {
        return sTexturePreviewStorage;
    }
}
