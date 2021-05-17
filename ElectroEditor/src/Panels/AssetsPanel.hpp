//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Renderer/Interface/Texture.hpp"
#include <glm/glm.hpp>

#define INPUT_BUFFER_LENGTH 128
namespace Electro
{
    struct DirectoryEntry
    {
        String Name;
        String Extension;
        String AbsolutePath;
        String ParentFolder;
        bool IsDirectory;

        virtual bool operator==(const DirectoryEntry& other) const { return AbsolutePath == other.AbsolutePath; }
        virtual bool operator!=(const DirectoryEntry& other) const { return !(*this == other); }
    };

    Ref<Texture2D>& GetTexturePreviewtorage();
    class AssetsPanel
    {
    public:
        AssetsPanel(const void* editorModulePtr);
        ~AssetsPanel() = default;

        void Init();
        void OnImGuiRender(bool* show);
    private:
        void RenderTexturePreviewPanel();
        void DrawPath(DirectoryEntry& entry);
        Vector<DirectoryEntry> GetFiles(const String& directory);
        void DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes);
        void UpdateSplitStringBuffer();
        void HandleExtension(DirectoryEntry& entry, RendererID texID);
        void HandleRenaming(DirectoryEntry& entry);
        const String SearchAssets(const String& query);
    private:
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
    };
}
