//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"
#include "IPanel.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    class AssetsPanel : public IPanel
    {
    public:
        AssetsPanel(void* editorModulePtr);
        ~AssetsPanel() = default;

        virtual void Init(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
        void Load();
    private:
        void DrawPath(DirectoryEntry& entry);
        void UpdateSplitStringBuffer();
        void HandleExtension(DirectoryEntry& entry, const RendererID texID);
        void StartRenaming();
        void HandleRenaming();
        void HandleDeleting(DirectoryEntry& entry);
        String SearchAssets(const String& query);
        [[nodiscard]] String EnsureExtension(const String& ext) const;
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
