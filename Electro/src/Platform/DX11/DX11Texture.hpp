//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroTexture.hpp"
#include <d3d11.h>

namespace Electro
{
    class DX11Texture2D : public Texture2D
    {
    public:
        DX11Texture2D(Uint width, Uint height);
        DX11Texture2D(const String& path, bool flipped = false);
        ~DX11Texture2D();
        virtual void Bind(Uint bindslot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const override;
        virtual const String GetName() const override { return m_Name; }
        virtual Uint GetWidth()  const override { return m_Width; }
        virtual Uint GetHeight() const override { return m_Height; }
        virtual String GetFilepath() const override { return m_Filepath; }
        virtual RendererID GetRendererID() const override { return (RendererID)m_SRV; }
        virtual void SetData(void* data, Uint size) override;
        virtual bool Loaded() override { return m_Loaded; };
        virtual void Reload(bool flip = false);
        virtual void Unbind() const override {}
        virtual bool operator ==(const Texture& other) const override { return m_SRV == ((DX11Texture2D&)other).m_SRV; }
    private:
        void LoadTexture(bool flip);
    private:
        ID3D11Texture2D*          m_Texture2D;
        ID3D11ShaderResourceView* m_SRV;

        int m_Width = 0;
        int m_Height = 0;
        String m_Filepath;
        String m_Name;
        bool m_Loaded = false;
    };

    class DX11TextureCube : public TextureCube
    {
    public:
        DX11TextureCube(const String& folderPath);
        ~DX11TextureCube();
        virtual void Bind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const override;
        virtual String GetFilepath() const override { return m_FilePath; }
        virtual Uint GetWidth() const override { return m_Width; }
        virtual Uint GetHeight() const override { return m_Height; }
        virtual String const GetName() const override { return m_Name; }
        virtual RendererID GetRendererID() const override { return m_RendererID; }
        virtual bool Loaded() override { return m_Loaded; }
        virtual void Reload(bool flip = false) override;
        virtual void SetData(void* data, Uint size) override {}
        virtual void Unbind() const override {}
        virtual bool operator ==(const Texture& other) const override { return m_RendererID == ((DX11TextureCube&)other).m_RendererID; }
    private:
        void LoadTextureCube(bool flip);
    private:
        String m_FilePath;
        Vector<String> m_Faces;
        RendererID m_RendererID;
        Uint m_Width, m_Height;
        String m_Name;
        bool m_Loaded = false;

        ID3D11ShaderResourceView* m_SRV;
    };
}
