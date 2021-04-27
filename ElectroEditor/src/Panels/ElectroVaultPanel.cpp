//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroVaultPanel.hpp"
#include "Core/ElectroVault.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroEditorLayer.hpp"
#include "ElectroUIMacros.hpp"
#include <imgui.h>
#include <filesystem>

namespace Electro
{
    static void* sEditorLayerStorage;
    static Ref<Texture2D> sTexturePreviewStorage;
    static bool sLoaded = false;

    VaultPanel::VaultPanel(const void* editorLayerPtr)
    {
        sEditorLayerStorage = (EditorLayer*)editorLayerPtr;
        mProjectPath.clear();
        sLoaded = false;
    }

    Vector<DirectoryEntry> VaultPanel::GetFiles(const String& directory)
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

    void VaultPanel::Init()
    {
        mFolderTextureID = Vault::Get<Texture2D>("Folder.png")->GetRendererID();
        mCSTextureID = Vault::Get<Texture2D>("CSharpIcon.png")->GetRendererID();
        mCPPTextureID = Vault::Get<Texture2D>("CPPIcon.png")->GetRendererID();
        mElectroTextureID = Vault::Get<Texture2D>("ElectroIcon.png")->GetRendererID();
        mUnknownTextureID = Vault::Get<Texture2D>("UnknownIcon.png")->GetRendererID();
        mShaderTextureID = Vault::Get<Texture2D>("ShaderIcon.png")->GetRendererID();
        m3DFileTextureID = Vault::Get<Texture2D>("3DFileIcon.png")->GetRendererID();
        sTexturePreviewStorage = Vault::Get<Texture2D>("Prototype.png");
    }

    void VaultPanel::OnImGuiRender(bool* show)
    {
        if (mProjectPath.empty())
        {
            mProjectPath = Vault::GetProjectPath();
            mDrawingPath = mProjectPath;
        }

        ImGui::Begin("ElectroVault", show);

        if (Vault::IsVaultInitialized() && ImGui::Button("Refresh"))
        {
            Vault::Reload();
            mProjectPath = Vault::GetProjectPath();
            mFiles = GetFiles(mProjectPath);
        }

        ImGui::SameLine();

        if (mProjectPath != mDrawingPath && ImGui::Button("Back"))
            mDrawingPath = OS::GetParentPath(mDrawingPath);

        ImGui::TextColored(UI::GetStandardColorImVec4(), mDrawingPath.c_str());

        if (!sLoaded && !mProjectPath.empty())
        {
            mFiles = GetFiles(mProjectPath);
            sLoaded = true;
        }

        if (Vault::IsVaultInitialized())
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

        ImGui::Begin("Texture Preview", false, ImGuiWindowFlags_HorizontalScrollbar);
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
            sTexturePreviewStorage = EDevice::CreateTexture2D(*(String*)data->Data);
            glm::vec2 imageRes = { sTexturePreviewStorage->GetWidth(), sTexturePreviewStorage->GetHeight() };
            ImVec2 windowRes = ImGui::GetWindowSize();
            DrawImageAtMiddle(imageRes, { windowRes.x, windowRes.y });
            UI::Image(sTexturePreviewStorage->GetRendererID(), { imageRes.x, imageRes.y });
        }
        ImGui::End();
    }

    void VaultPanel::DrawPath(DirectoryEntry& entry)
    {
        const float itemSize = 50.0f;
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
            UI::DragAndDropSource(ELECTRO_SCENE_FILE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop at Viewport to open the scene");
        }
        else if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".bmp" || entry.Extension == ".tga" || entry.Extension == ".hdr")
        {
            if (UI::ImageButton(Vault::Get<Texture2D>(entry.Name + entry.Extension)->GetRendererID(), { itemSize, itemSize }, buttonBackgroundColor))
            {
                if (sTexturePreviewStorage)
                    sTexturePreviewStorage = nullptr;
                sTexturePreviewStorage = EDevice::CreateTexture2D(entry.AbsolutePath);
                ImGui::SetWindowFocus("Texture Preview");
            }
            UI::DragAndDropSource(TEXTURE_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Texture is needed, to set this Texture");
        }
        else if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".dae" || entry.Extension == ".gltf" || entry.Extension == ".3ds")
        {
            UI::ImageButton(m3DFileTextureID, { itemSize, itemSize }, buttonBackgroundColor);
            UI::DragAndDropSource(MESH_DND_ID, &entry.AbsolutePath, sizeof(entry.AbsolutePath), "Drop somewhere where Mesh is needed, to set this Mesh");
        }
        else if (entry.Extension == ".cs")
            UI::ImageButton(mCSTextureID, { itemSize, itemSize }, buttonBackgroundColor);
        else if (entry.Extension == ".cpp")
            UI::ImageButton(mCPPTextureID, { itemSize, itemSize }, buttonBackgroundColor);
        else if (entry.Extension == ".hlsl" || entry.Extension == ".glsl")
            UI::ImageButton(mShaderTextureID, { itemSize, itemSize }, buttonBackgroundColor);
        else
            UI::ImageButton(mUnknownTextureID, { itemSize, itemSize }, buttonBackgroundColor);

        ImGui::TextWrapped((entry.Name + entry.Extension).c_str());
    }
    void VaultPanel::DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes)
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
