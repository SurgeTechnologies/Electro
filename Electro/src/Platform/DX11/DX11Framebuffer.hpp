//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroFramebuffer.hpp"
#include <d3d11.h>
#include <wrl.h>

namespace Electro
{
    class DX11Framebuffer : public Framebuffer
    {
    public:
        DX11Framebuffer(const FramebufferSpecification& spec);
        virtual ~DX11Framebuffer() = default;

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual void Resize(Uint width, Uint height) override;
        virtual FramebufferSpecification& GetSpecification() override { return mSpecification; }
        virtual void Clear(const glm::vec4& clearColor) override;
        virtual RendererID GetColorViewID() override { return (RendererID)mSRV.Get(); }
        virtual RendererID GetSwapChainTarget() override;
    private:
        void CreateSwapChainView();
        void CreateColorView(FramebufferSpecification::BufferDesc desc);
        void CreateDepthView(FramebufferSpecification::BufferDesc desc);
        bool IsDepthFormat(const FormatCode format);

        void Invalidate();
        void Clean();
    private:
        FramebufferSpecification mSpecification;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> mRenderTargetTexture;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;

        bool mIsDepth = false;
        D3D11_VIEWPORT mViewport;
    };
}
