//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroAssetsPanel.hpp"
#include "Asset/ElectroAssetManager.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroEditorModule.hpp"
#include "ElectroUIMacros.hpp"
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
            AssetManager::Reload();
            mProjectPath = AssetManager::GetProjectPath();
            mFiles = GetFiles(mProjectPath);
        }

        ImGui::SameLine();

        if (mProjectPath != mDrawingPath && ImGui::Button(ICON_ELECTRO_BACKWARD))
            mDrawingPath = OS::GetParentPath(mDrawingPath);

        ImGui::TextColored(UI::GetStandardColorImVec4(), mDrawingPath.c_str());

        if (!sLoaded && !mProjectPath.empty())
        {
            mFiles = GetFiles(mProjectPath);
            sLoaded = true;
        }

        if (AssetManager::IsInitialized())
        {
            const float itemSize = 60.0f;
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
            sTexturePreviewStorage = EGenerator::CreateTexture2D(*(String*)data->Data);
            glm::vec2 imageRes = { sTexturePreviewStorage->GetWidth(), sTexturePreviewStorage->GetHeight() };
            ImVec2 windowRes = ImGui::GetWindowSize();
            DrawImageAtMiddle(imageRes, { windowRes.x, windowRes.y });
            UI::Image(sTexturePreviewStorage->GetRendererID(), { imageRes.x, imageRes.y });
        }
        ImGui::End();
    }

    void AssetsPanel::DrawPath(DirectoryEntry& entry)
    {
        const float itemSize = 50.0f;
        bool deleted = false;
        ImGui::TableNextColumn();
        ImVec4 buttonBackgroundColor = ImVec4(0.176470f, 0.176470f, 0.176470f, 1.0f);
        if (entry.IsDirectory)
        {
            if (UI::ImageButton(mFolderTextureID, { itemSize, itemSize }, buttonBackgroundColor))
                mDrawingPath = entry.AbsolutePath;
        }
        else if (entry.Extension == ".electro")
        {
            UI::ImageButton(mElectroTextureID, { itemSize, itemSize }, buttonBackgroundColor);
            if (ImGui::IsItemClicked(1))
                ImGui::OpenPopup("Electro Settings");
            if (ImGui::BeginPopup("Electro Settings"))
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (OS::AMessageBox("Are you sure you want to delete this file?", "Deleting this file will completely remove it form this machine.", DialogType::Yes__No, IconType::Warning, DefaultButton::No))
                    {
                        OS::Deletefile(entry.AbsolutePath.c_str());
                        mFiles = GetFiles(mProjectPath);
                        deleted = true;
                    }
                }
                ImGui::EndPopup();
            }
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop at Viewport to open the scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            if (UI::ImageButton(AssetManager::Get<Texture2D>(entry.Name + entry.Extension)->GetRendererID(), { itemSize, itemSize }, buttonBackgroundColor))
            {
                if (sTexturePreviewStorage)
                    sTexturePreviewStorage = nullptr;
                sTexturePreviewStorage = EGenerator::CreateTexture2D(entry.AbsolutePath);
                ImGui::SetWindowFocus(TEXTURE_PREVIEW_TITLE);
            }
            if (ImGui::IsItemClicked(1))
                ImGui::OpenPopup("Image Settings");

            if (ImGui::BeginPopup("Image Settings"))
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (OS::AMessageBox("Are you sure you want to delete this file?", "Deleting this file will completely remove it form this machine.", DialogType::Yes__No, IconType::Warning, DefaultButton::No))
                    {
                        OS::Deletefile(entry.AbsolutePath.c_str());
                        mFiles = GetFiles(mProjectPath);
                        deleted = true;
                    }
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.8)
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
                ImGui::TextUnformatted("Electro Texture | Status: Cached");
                ImGui::Text("Type: %s", entry.Extension.c_str());
                ImGui::Text("Size: %f MB", OS::GetFileSize(entry.AbsolutePath.c_str()));
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Texture is needed, to set this Texture");
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds")
        {
            UI::ImageButton(m3DFileTextureID, { itemSize, itemSize }, buttonBackgroundColor);
            if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.8)
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
                ImGui::Text("Type: %s", entry.Extension.c_str());
                ImGui::Text("Size: %f MB", OS::GetFileSize(entry.AbsolutePath.c_str()));
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Mesh is needed, to set this Mesh");
        }
        else if (entry.Extension == ".cs")
        {
            UI::ImageButton(mCSTextureID, { itemSize, itemSize }, buttonBackgroundColor);
            if (ImGui::IsItemClicked(1))
                ImGui::OpenPopup("CS Settings");
            if (ImGui::BeginPopup("CS Settings"))
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (OS::AMessageBox("Are you sure you want to delete this file?", "Deleting this file will completely remove it form this machine.", DialogType::Yes__No, IconType::Warning, DefaultButton::No))
                    {
                        OS::Deletefile(entry.AbsolutePath.c_str());
                        mFiles = GetFiles(mProjectPath);
                        deleted = true;
                    }
                }
                ImGui::EndPopup();
            }
        }
        else
        {
            UI::ImageButton(mUnknownTextureID, { itemSize, itemSize }, buttonBackgroundColor);
            if (ImGui::IsItemClicked(1))
                ImGui::OpenPopup("UnknownSettings");
            if (ImGui::BeginPopup("UnknownSettings"))
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (OS::AMessageBox("Are you sure you want to delete this file?", "Deleting this file will completely remove it form this machine.", DialogType::Yes__No, IconType::Warning, DefaultButton::No))
                    {
                        OS::Deletefile(entry.AbsolutePath.c_str());
                        mFiles = GetFiles(mProjectPath);
                        deleted = true;
                    }
                }
                ImGui::EndPopup();
            }
        }
        if(!deleted)
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
