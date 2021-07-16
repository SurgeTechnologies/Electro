//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"
#include "IPanel.hpp"
#include "Renderer/Interface/Texture.hpp"

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
        AssetsPanel();
        ~AssetsPanel() = default;

        virtual void OnInit(void* data) override;
        virtual void OnImGuiRender(bool* show) override;
    private:
        E_FORCE_INLINE void ChangeCurrentPath(const String& path) { mCurrentPath = path; std::replace(mCurrentPath.begin(), mCurrentPath.end(), '\\', '/'); }
        void DrawPath(const DirectoryEntry& entry);
        const Vector<DirectoryEntry> GetFiles(const String& directory);
    private:
        String mCurrentPath;
        bool mSkipText = false;
        Vector<DirectoryEntry> mFiles;

        Ref<Texture2D> mFolderTextureID;
        Ref<Texture2D> mUnknownTextureID;
        Ref<Texture2D> m3DFileTextureID;
        Ref<Texture2D> mImageTextureID;
    };
}
