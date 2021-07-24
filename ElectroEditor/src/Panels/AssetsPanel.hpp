//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/Interface/Texture.hpp"
#include "Asset/AssetManager.hpp"
#include "Utility/StringUtils.hpp"
#include "IPanel.hpp"
#include "UIMacros.hpp"

namespace Electro
{
    struct DirectoryEntry
    {
        String Name;
        String NameWithExtension;
        String Extension;
        String AbsolutePath;
        String ParentFolder;
        bool IsDirectory;
    };

    struct AssetDropData
    {
        AssetHandle Handle;
        const char* Path;
    };

    class AssetsPanel : public IPanel
    {
    public:
        AssetsPanel(void* editorModulePtr);
        ~AssetsPanel() = default;

        virtual void OnInit(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
        void Load();
    private:
        void DrawPath(const DirectoryEntry& entry);
        void UpdateSplitStringBuffer();
        void HandleExtension(const DirectoryEntry& entry, const RendererID texID);
        void HandleDeleting(const DirectoryEntry& entry);
        String SearchAssets(const String& query);
        Vector<DirectoryEntry> GetFiles(const String& directory);
    private:
        bool mAssetsPanelFocused = false;
        bool mSkipText = false;

        String mDrawingPath;
        String mSearchBuffer;
        char mNameBuffer[INPUT_BUFFER_LENGTH];

        DirectoryEntry mSelectedEntry;
        Vector<DirectoryEntry> mFiles;
        Vector<String> mSplitBuffer;
        Vector<String> mTempSplitBuffer;

        Ref<Texture2D> mFolderTex;
        Ref<Texture2D> mImageTex;
        Ref<Texture2D> m3DFileTex;
        Ref<Texture2D> mElectroTex;
        Ref<Texture2D> mUnknownTex;
        Ref<Texture2D> mMaterialTex;
        Ref<Texture2D> mPhysicsMatTex;
    };
}
