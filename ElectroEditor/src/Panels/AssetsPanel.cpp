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
        Vector<DirectoryEntry> result;
        try
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
            {
                String path = entry.path().string();
                DirectoryEntry e = { entry.path().stem().string(), entry.path().extension().string(), path, entry.path().parent_path().string(), entry.is_directory() };
                result.push_back(e);
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ELECTRO_ERROR("%s !", e.what());
        }
        catch (...)
        {
            ELECTRO_ERROR("Error on filesystem!");
        }
        return result;
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
            mDrawingPath = mProjectPath;
        UI::ToolTip("Go to Project Root directory");

        ImGui::SameLine();
        ImGui::TextUnformatted("|");

        const Vector<String>& strings = Utils::SplitString(mDrawingPath, "/\\");
        for (const String& str : strings)
        {
            ImGui::SameLine();
            if (ImGui::Button(str.c_str()))
                mDrawingPath = mDrawingPath.substr(0, mDrawingPath.find(str) + str.size());

            ImGui::SameLine();
            ImGui::TextColored(UI::GetStandardColorImVec4(), "/");
        }
        ImGui::Separator();

        if (!sLoaded && !mProjectPath.empty())
        {
            mFiles = GetFiles(mProjectPath);
            sLoaded = true;
        }

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
                mDrawingPath = entry.AbsolutePath;
        }
        else if (entry.Extension == ".electro")
        {
            UI::ImageButton(mElectroTextureID, { width, height }, buttonBackgroundColor);
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop at Viewport to open this scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            UI::ImageButton(mImageTextureID, { width, height }, buttonBackgroundColor);
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Texture is needed");
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds")
        {
            UI::ImageButton(m3DFileTextureID, { width, height }, buttonBackgroundColor);
            Pair<String, String> data = { entry.Name, entry.AbsolutePath };
            UI::DragAndDropSource(MESH_DND_ID, &data, sizeof(data), "Drop somewhere where Mesh is needed");
        }
        else if (entry.Extension == ".cs")
        {
            UI::ImageButton(mCSTextureID, { width, height }, buttonBackgroundColor);
            UI::DragAndDropSource(READABLE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop in " CODE_EDITOR_TITLE " to open this file");
        }
        else
            UI::ImageButton(mUnknownTextureID, { width, height }, buttonBackgroundColor);

        ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
    }

    void AssetsPanel::DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes)
    {
        glm::vec2 imageMiddle = { imageRes.x * 0.5f, imageRes.y * 0.5f };
        glm::vec2 windowMiddle = { windowRes.x * 0.5f, windowRes.y * 0.5f };
        glm::vec2 result = { windowMiddle - imageMiddle };
        ImGui::SetCursorPos({ result.x, result.y });
    }

    Ref<Texture2D>& Electro::GetTexturePreviewtorage()
    {
        return sTexturePreviewStorage;
    }
}
