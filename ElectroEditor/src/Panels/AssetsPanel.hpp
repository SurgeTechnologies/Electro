//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Renderer/Interface/Texture.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    struct DirectoryEntry
    {
        String Name;
        String Extension;
        String AbsolutePath;
        String ParentFolder;
        bool IsDirectory;
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
        void DrawPath(DirectoryEntry& entry);
        Vector<DirectoryEntry> GetFiles(const String& directory);
        void DrawImageAtMiddle(const glm::vec2& imageRes, const glm::vec2& windowRes);
        void UpdateSplitStringBuffer();
        const String SearchAssets(const String& query);
    private:
        String mProjectPath;
        String mDrawingPath;
        String mSearchBuffer;

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
