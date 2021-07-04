//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Texture.hpp"
#include "Renderer/Interface/Shader.hpp"
#include <d3d11.h>
#include <array>

namespace Electro
{
    class DX11Texture2D : public Texture2D
    {
    public:
        DX11Texture2D(const Texture2DSpecification& spec);
        ~DX11Texture2D();

        virtual const Texture2DSpecification& GetSpecification() const override { return mSpecification; }
        virtual void Bind(Uint slot, const ShaderDomain shaderDomain) const override;
        virtual void Unbind(Uint slot, const ShaderDomain shaderDomain) const override;
        virtual bool IsHDR() const override { return mIsHDR; }
        virtual Uint GetWidth() const override { return mWidth; }
        virtual Uint GetHeight() const override { return mHeight; }
        virtual RendererID GetRendererID() const override { return mSRV; }
        virtual bool Loaded() override { return mLoaded; }
    private:
        void Load();
        void LoadDataAndSetFormat(D3D11_TEXTURE2D_DESC& desc);
    private:
        void* mImageData = nullptr;
        bool mLoaded = false;
        bool mIsHDR = false;
        Uint mWidth = 0, mHeight = 0;
        Texture2DSpecification mSpecification;

        ID3D11ShaderResourceView* mSRV = nullptr;
        ID3D11ShaderResourceView* mNullSRV = nullptr;
    };

    class DX11Cubemap : public Cubemap
    {
    public:
        DX11Cubemap(const String& path);
        ~DX11Cubemap();
        virtual void VSBind(Uint slot = 0) const override;
        virtual void PSBind(Uint slot = 0) const override;
        virtual void CSBind(Uint slot = 0) const override;
        virtual void Unbind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const override;
        virtual RendererID GenIrradianceMap() override;
        virtual RendererID GenPreFilter() override;
        virtual void BindIrradianceMap(Uint slot) const override;
        virtual void BindPreFilterMap(Uint slot) const override;
        virtual String GetPath() const override { return mPath; }
        virtual String const GetName() const override { return mName; }
        virtual RendererID GetRendererID() const override { return (RendererID)mSRV; }
        virtual Uint CalculateMipMapCount(Uint width, Uint height) override;
        virtual bool operator ==(const Cubemap& other) const override { return mSRV == ((DX11Cubemap&)other).mSRV; }
    private:
        void LoadCubemap();
    private:
        String mPath;
        String mName;
        std::array<glm::mat4, 6> mCaptureViewProjection;
        ID3D11ShaderResourceView* mSRV = nullptr;
        ID3D11ShaderResourceView* mIrradianceSRV = nullptr;
        ID3D11ShaderResourceView* mPreFilterSRV = nullptr;
        ID3D11ShaderResourceView* mNullSRV = nullptr;
    };
}
