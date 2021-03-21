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
        virtual FramebufferSpecification& GetSpecification() override { return m_Specification; }
        virtual void Clear(const glm::vec4& clearColor) override;
        virtual RendererID GetColorViewID() override { return (RendererID)m_SRV.Get(); }
        virtual RendererID GetSwapChainTarget() override;
    private:
        void CreateSwapChainView();
        void CreateColorView(FramebufferSpecification::BufferDesc desc);
        void CreateDepthView(FramebufferSpecification::BufferDesc desc);
        bool IsDepthFormat(const FormatCode format);

        void Invalidate();
        void Clean();
    private:
        FramebufferSpecification m_Specification;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RenderTargetTexture;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

        bool m_IsDepth = false;
        D3D11_VIEWPORT mViewport;
    };
}
