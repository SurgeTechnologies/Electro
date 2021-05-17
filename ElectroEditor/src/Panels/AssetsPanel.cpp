//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "AssetsPanel.hpp"
#include "Asset/AssetManager.hpp"
#include "Core/System/OS.hpp"
#include "Core/FileSystem.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utility/StringUtils.hpp"
#include "UIUtils/UIUtils.hpp"
#include "EditorModule.hpp"
#include "UIMacros.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

namespace Electro
{
    static EditorModule* sEditorModuleStorage;
    static Ref<Texture2D> sTexturePreviewStorage;
    static bool sLoaded = false;

    AssetsPanel::AssetsPanel(const void* editorModulePtr)
    {
        sEditorModuleStorage = (EditorModule*)editorModulePtr;
        mProjectPath.clear();
        sLoaded = false;
    }

    Vector<DirectoryEntry> AssetsPanel::GetFiles(const String& directory)
    {
        Deque<DirectoryEntry> result;
        try
        {
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
            {
                const std::filesystem::path& path = entry.path();
                DirectoryEntry e;
                e.Name = path.stem().string();
                e.Extension = path.extension().string();
                e.AbsolutePath = path.string();
                e.ParentFolder = path.parent_path().string();
                e.IsDirectory = entry.is_directory();
                if(e.IsDirectory)
                    result.push_front(e);
                else
                    result.push_back(e);
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ELECTRO_ERROR("%s!", e.what());
        }
        catch (...)
        {
            ELECTRO_ERROR("Error on filesystem(While trying to get files from OS)!");
        }
        Vector<DirectoryEntry> vecResult;
        vecResult.resize(result.size());

        for (Uint i = 0; i < result.size(); i++)
        {
            vecResult.emplace_back(std::move(result[i]));
        }
        result.clear();
        return vecResult;
    }

    void AssetsPanel::Init()
    {
        mFolderTextureID  = AssetManager::Get<Texture2D>("Folder.png")->GetRendererID();
        mCSTextureID      = AssetManager::Get<Texture2D>("CSharpIcon.png")->GetRendererID();
        mElectroTextureID = AssetManager::Get<Texture2D>("ElectroIcon.png")->GetRendererID();
        mUnknownTextureID = AssetManager::Get<Texture2D>("UnknownIcon.png")->GetRendererID();
        m3DFileTextureID  = AssetManager::Get<Texture2D>("3DFileIcon.png")->GetRendererID();
        mImageTextureID   = AssetManager::Get<Texture2D>("ImageIcon.png")->GetRendererID();
        sTexturePreviewStorage = AssetManager::Get<Texture2D>("Prototype.png");
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
            mFiles = GetFiles(mProjectPath);
            sLoaded = true;
        }

        ImGui::Begin(ASSETS_TITLE, show);

        if (AssetManager::IsInitialized() && ImGui::Button("Refresh"))
        {
            mProjectPath = AssetManager::GetProjectPath();
            mFiles = GetFiles(mProjectPath);
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

        //TODO
        ImGui::Button("Create");
        UI::ToolTip("Create assets!");
        ImGui::SameLine();

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
            UI::DrawRectAroundWidget(UI::GetStandardColorGLMVec4());
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
            if (ImGui::BeginTable("##VaultTable", columns, ImGuiTableFlags_SizingFixedSame))
            {
                ImGui::TableSetupColumn("##VaultColumn", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, itemSize);
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

    void AssetsPanel::DrawPath(DirectoryEntry& entry)
    {
        const float width = 50.0f;
        const float height = 50.0f;

        ImGui::TableNextColumn();
        ImVec4& buttonBackgroundColor = ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f);
        if (entry.IsDirectory)
        {
            if (UI::ImageButton(mFolderTextureID, { width, height }, buttonBackgroundColor))
            {
                mDrawingPath = entry.AbsolutePath;
                UpdateSplitStringBuffer();
            }
        }
        else if (entry.Extension == ".electro")
        {
            if (UI::ImageButton(mElectroTextureID, { width, height }, buttonBackgroundColor))
                UpdateSplitStringBuffer();
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop at Viewport to open this scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            if (UI::ImageButton(mImageTextureID, { width, height }, buttonBackgroundColor))
                UpdateSplitStringBuffer();
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Texture is needed");
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds" || entry.Extension == ".FBX")
        {
            if (UI::ImageButton(m3DFileTextureID, { width, height }, buttonBackgroundColor))
                UpdateSplitStringBuffer();
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Mesh is needed");
        }
        else if (entry.Extension == ".cs")
        {
            if (UI::ImageButton(mCSTextureID, { width, height }, buttonBackgroundColor))
                UpdateSplitStringBuffer();
            UI::DragAndDropSource(READABLE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop in " CODE_EDITOR_TITLE " to open this file");
        }
        else
        {
            if (UI::ImageButton(mUnknownTextureID, { width, height }, buttonBackgroundColor))
                UpdateSplitStringBuffer();
        }

        ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
    }

    void AssetsPanel::DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes)
    {
        glm::vec2 imageMiddle = { imageRes.x * 0.5f, imageRes.y * 0.5f };
        glm::vec2 windowMiddle = { windowRes.x * 0.5f, windowRes.y * 0.5f };
        glm::vec2 result = { windowMiddle - imageMiddle };
        ImGui::SetCursorPos({ result.x, result.y });
    }

    void AssetsPanel::UpdateSplitStringBuffer()
    {
        mSplitBuffer.clear();
        mSplitBuffer = Utils::SplitString(mDrawingPath, "/\\");
    }

    const String AssetsPanel::SearchAssets(const String& query)
    {
        for (DirectoryEntry& file : mFiles)
        {
            if(file.IsDirectory)
                continue;

            if ((file.Name + file.Extension).find(query) != std::string::npos)
                return file.ParentFolder;
        }
        return mProjectPath;
    }

    Ref<Texture2D>& Electro::GetTexturePreviewtorage()
    {
        return sTexturePreviewStorage;
    }
}
