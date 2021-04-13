//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroVaultPanel.hpp"
#include "Core/ElectroVault.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Scene/ElectroSceneSerializer.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"
#include "UIUtils/ElectroUIUtils.hpp"
#include "ElectroEditorLayer.hpp"
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
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                String path = entry.path().string();
                DirectoryEntry e = { entry.path().stem().string(), entry.path().extension().string(), path, entry.is_directory() };

                if (entry.is_directory())
                    e.SubEntries = GetFiles(entry.path().string());
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

    void VaultPanel::OnImGuiRender(bool* show)
    {
        if (mProjectPath.empty())
            mProjectPath = Vault::GetProjectPath();

        ImGui::Begin("ElectroVault", show);
        if (UI::ColorButton(ICON_ELECTRO_REFRESH, UI::GetStandardColorImVec4()))
        {
            if (Vault::IsVaultInitialized())
            {
                Vault::Reload();
                mProjectPath = Vault::GetProjectPath();
                mFiles = GetFiles(mProjectPath);
            }
            else
                ELECTRO_WARN("Open A working directory first! Go to 'File>Open Folder' to open a working directory.");
        }

        if (!sLoaded && !mProjectPath.empty())
        {
            mFiles = GetFiles(mProjectPath);
            sLoaded = true;
        }

        if (Vault::IsVaultInitialized())
            for (auto& file : mFiles)
                DrawPath(file);
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
        else
        {
            ImVec2 windowRes = ImGui::GetWindowSize();
            ImGui::SetCursorPos({ windowRes.x * 0.2f, windowRes.y * 0.5f });
            ImGui::TextUnformatted("No Texture is selected. Select an image file\nin the Spike Vault to show it up here!");
        }
        ImGui::End();

        ImGui::Begin("Cache");
        if (ImGui::TreeNode("Shaders"))
        {
            auto& shaders = Vault::GetAllShaders();
            for (auto& shader : shaders)
                if (shader)
                    if (ImGui::TreeNode(shader->GetName().c_str()))
                        ImGui::TreePop();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Textures"))
        {
            auto& textures = Vault::GetAllTextures();
            for (auto& texture : textures)
                if (texture)
                    if (ImGui::TreeNode(texture->GetName().c_str()))
                        ImGui::TreePop();
            ImGui::TreePop();
        }
        ImGui::End();
    }

    void VaultPanel::DrawPath(DirectoryEntry& entry)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

        if (!entry.IsDirectory)
            flags |= ImGuiTreeNodeFlags_Leaf;

        String nodeString;

        bool codeExtBools = entry.Extension == ".cs" || entry.Extension == ".glsl" || entry.Extension == ".cpp" || entry.Extension == ".lua"
            || entry.Extension == ".py" || entry.Extension == ".hlsl" || entry.Extension == ".js" || entry.Extension == ".c" || entry.Extension == ".h";

        bool imageExtBools = entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".gif"
            || entry.Extension == ".bmp" || entry.Extension == ".psd";

        if (codeExtBools)
            nodeString = ICON_ELECTRO_CODE + String(" ") + entry.Name + entry.Extension;
        else if (entry.Extension == ".electro" || entry.Extension == ".txt")
            nodeString = ICON_ELECTRO_FILE_TEXT_O + String(" ") + entry.Name + entry.Extension;
        else if (imageExtBools)
            nodeString = ICON_ELECTRO_FILE_IMAGE_O + String(" ") + entry.Name + entry.Extension;
        else if (entry.IsDirectory)
            nodeString = ICON_ELECTRO_FOLDER + String(" ") + entry.Name;
        else nodeString = entry.Name + entry.Extension;

        if (ImGui::TreeNodeEx(nodeString.c_str(), flags))
        {
            if (entry.IsDirectory)
                for (auto& subDirectory : entry.SubEntries)
                    DrawPath(subDirectory);

            //Loading Electro files
            if (entry.Extension == ".electro" && ImGui::IsItemClicked(0))
            {
                int filepath = OS::AMessageBox("", "Do you want to open this scene?", DialogType::Yes__No, IconType::Question, DefaultButton::No);
                if(filepath)
                {
                    ((EditorLayer*)sEditorLayerStorage)->mActiveFilepath = entry.AbsolutePath;
                    ((EditorLayer*)sEditorLayerStorage)->InitSceneEssentials();

                    SceneSerializer serializer(((EditorLayer*)sEditorLayerStorage)->mEditorScene, ((EditorLayer*)sEditorLayerStorage));
                    serializer.Deserialize(entry.AbsolutePath);
                    ((EditorLayer*)sEditorLayerStorage)->UpdateWindowTitle(OS::GetNameWithoutExtension(entry.AbsolutePath));
                }
            }

            //Texture
            if (imageExtBools && ImGui::IsItemClicked(0))
            {
                if (sTexturePreviewStorage)
                    sTexturePreviewStorage = nullptr;
                sTexturePreviewStorage = Texture2D::Create(entry.AbsolutePath);
                ImGui::SetWindowFocus("Texture Preview");
            }
            ImGui::TreePop();
        }
    }

    void VaultPanel::DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes)
    {
        glm::vec2 imageMiddle = { imageRes.x * 0.5f, imageRes.y * 0.5f };
        glm::vec2 windowMiddle = { windowRes.x * 0.5f, windowRes.y * 0.5f };
        glm::vec2 result = { windowMiddle - imageMiddle };
        ImGui::SetCursorPos({ result.x, result.y });
    }
}
