//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"
#include "IPanel.hpp"
#include "Renderer/Interface/Texture.hpp"
#include <glm/glm.hpp>

#define INPUT_BUFFER_LENGTH 128
namespace Electro
{
    class AssetsPanel : public IPanel
    {
    public:
        AssetsPanel(const void* editorModulePtr);
        ~AssetsPanel() = default;

        virtual void Init(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
    private:
        void DrawPath(DirectoryEntry& entry);
        void DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes);
        void UpdateSplitStringBuffer();
        void HandleExtension(DirectoryEntry& entry, RendererID texID);
        void StartRenaming();
        void HandleRenaming(DirectoryEntry& entry);
        void HandleDeleting(DirectoryEntry& entry);
        const String SearchAssets(const String& query);
        String EnsureExtension(const String& ext) const;
    private:
        bool mAssetsPanelFocused = false;
        bool mRenaming;
        bool mSkipText = false;

        String mProjectPath;
        String mDrawingPath;
        String mSearchBuffer;
        char mRenameBuffer[INPUT_BUFFER_LENGTH];

        DirectoryEntry mSelectedEntry;
        Vector<DirectoryEntry> mFiles;
        Vector<String> mSplitBuffer;
        Vector<String> mTempSplitBuffer;

        RendererID mFolderTextureID;
        RendererID mCSTextureID;
        RendererID mImageTextureID;
        RendererID m3DFileTextureID;
        RendererID mElectroTextureID;
        RendererID mUnknownTextureID;
        RendererID mMaterialTextureID;
        RendererID mPhysicsMatTextureID;
    };
}
