//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Framebuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    // Max: 8K Texture; 8192 / 1204 = 8
    static const Uint sMaxFramebufferSize = 8192;

    namespace Utils
    {
        static void AttachColorTexture(FramebufferTextureSpecification textureSpec, FramebufferSpecification framebufferSpec, FramebufferColorAttachment* outColorAttachment)
        {
            ID3D11Device* device = DX11Internal::GetDevice();

            // Render Target texture
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = framebufferSpec.Width;
            textureDesc.Height = framebufferSpec.Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &outColorAttachment->RenderTargetTexture));

            // Render Target View
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;
            DX_CALL(device->CreateRenderTargetView(outColorAttachment->RenderTargetTexture.Get(), &renderTargetViewDesc, &outColorAttachment->RenderTargetView));

            // Shader Resource View
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
            shaderResourceViewDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;
            DX_CALL(device->CreateShaderResourceView(outColorAttachment->RenderTargetTexture.Get(), &shaderResourceViewDesc, &outColorAttachment->ShaderResourceView));
        }

        static void AttachDepthTexture(FramebufferTextureSpecification textureSpec, FramebufferSpecification framebufferSpec, FramebufferDepthAttachment* outDepthAttachment)
        {
            ID3D11Device* device = DX11Internal::GetDevice();

            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = framebufferSpec.Width;
            textureDesc.Height = framebufferSpec.Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            if (static_cast<DXGI_FORMAT>(textureSpec.TextureFormat) == DXGI_FORMAT_R32_TYPELESS)
                textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &outDepthAttachment->DepthStencilBuffer));

            D3D11_DEPTH_STENCIL_DESC dsDesc;
            dsDesc.DepthEnable = true;
            dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

            // Stencil test parameters
            dsDesc.StencilEnable = true;
            dsDesc.StencilReadMask = 0xFF;
            dsDesc.StencilWriteMask = 0xFF;

            dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

            dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
            dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
            DX_CALL(device->CreateDepthStencilState(&dsDesc, &outDepthAttachment->DepthStencilState));

            if (static_cast<DXGI_FORMAT>(textureSpec.TextureFormat) == DXGI_FORMAT_R32_TYPELESS) // Shadows
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
                shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
                shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
                shaderResourceViewDesc.Texture2D.MipLevels = 1;
                DX_CALL(device->CreateShaderResourceView(outDepthAttachment->DepthStencilBuffer.Get(), &shaderResourceViewDesc, &outDepthAttachment->ShaderResourceView));

                D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
                depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
                depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                depthStencilViewDesc.Texture2D.MipSlice = 0;
                DX_CALL(device->CreateDepthStencilView(outDepthAttachment->DepthStencilBuffer.Get(), &depthStencilViewDesc, &outDepthAttachment->DepthStencilView));
            }
            else // Normal
            {
                D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
                depthStencilViewDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
                depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                depthStencilViewDesc.Texture2D.MipSlice = 0;
                DX_CALL(device->CreateDepthStencilView(outDepthAttachment->DepthStencilBuffer.Get(), &depthStencilViewDesc, &outDepthAttachment->DepthStencilView));
            }
        }

        static void AttachToSwapchain(FramebufferColorAttachment* outColorAttachment)
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

            ID3D11Device* device = DX11Internal::GetDevice();
            IDXGISwapChain* swapChain = DX11Internal::GetSwapChain();
            swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
            device->CreateRenderTargetView(backBuffer.Get(), nullptr, &outColorAttachment->RenderTargetView);
        }

        static bool IsDepthFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::D24_UNORM_S8_UINT: return true;
                case FramebufferTextureFormat::R32_TYPELESS: return true;
            }

            return false;
        }
    }

    DX11Framebuffer::DX11Framebuffer(const FramebufferSpecification& spec)
        : mSpecification(spec)
    {
        for (auto format : mSpecification.Attachments.Attachments)
        {
            if (!Utils::IsDepthFormat(format.TextureFormat))
                mColorAttachmentSpecifications.emplace_back(format);
            else
                mDepthAttachmentSpecification = format;
        }

        Invalidate();
    }

    void DX11Framebuffer::Bind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->RSSetViewports(1, &mViewport);

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderViews[2];
        for (Uint i = 0; i < mColorAttachments.size(); i++)
            pRenderViews[i] = mColorAttachments[i].RenderTargetView;

        deviceContext->OMSetRenderTargets(static_cast<Uint>(mColorAttachments.size()), pRenderViews[0].GetAddressOf(), mDepthAttachment.DepthStencilView.Get());

        if (mDepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
            deviceContext->OMSetDepthStencilState(mDepthAttachment.DepthStencilState.Get(), 1);
    }

    void DX11Framebuffer::BindColorBufferAsTexture(Uint index, Uint slot) const
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, mColorAttachments[index].ShaderResourceView.GetAddressOf());
    }

    void DX11Framebuffer::UnbindColorBufferAsTexture(Uint slot) const
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mNullSRV);
    }

    void DX11Framebuffer::BindDepthBufferAsTexture(Uint slot) const
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, mDepthAttachment.ShaderResourceView.GetAddressOf());
    }

    void DX11Framebuffer::UnbindDepthBufferAsTexture(Uint slot) const
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mNullSRV);
    }

    void DX11Framebuffer::Invalidate()
    {
        Clean();
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(mSpecification.Width);
        mViewport.Height = static_cast<float>(mSpecification.Height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        bool multiSample = mSpecification.Samples > 1;

        if (mColorAttachmentSpecifications.size())
        {
            mColorAttachments.resize(mColorAttachmentSpecifications.size());

            // Attachments
            for (size_t i = 0; i < mColorAttachments.size(); i++)
            {
                if (mSpecification.SwapChainTarget)
                {
                    Utils::AttachToSwapchain(&mColorAttachments[i]); break;
                }

                switch (mColorAttachmentSpecifications[i].TextureFormat)
                {
                    case FramebufferTextureFormat::RGBA32F:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                    case FramebufferTextureFormat::R8G8B8A8_UNORM:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                    case FramebufferTextureFormat::R32_SINT:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                }
            }
        }

        if (mDepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
        {
            switch (mDepthAttachmentSpecification.TextureFormat)
            {
            case FramebufferTextureFormat::D24_UNORM_S8_UINT:
                Utils::AttachDepthTexture(mDepthAttachmentSpecification, mSpecification, &mDepthAttachment); break;
            case FramebufferTextureFormat::R32_TYPELESS:
                Utils::AttachDepthTexture(mDepthAttachmentSpecification, mSpecification, &mDepthAttachment); break;
            }
        }
    }

    void DX11Framebuffer::Clean()
    {
        for (size_t i = 0; i < mColorAttachments.size(); i++)
        {
            mColorAttachments[i].RenderTargetTexture.Reset();
            mColorAttachments[i].RenderTargetView.Reset();
            mColorAttachments[i].ShaderResourceView.Reset();
        }
        mColorAttachments.clear();

        mDepthAttachment.DepthStencilView.Reset();
        mDepthAttachment.DepthStencilState.Reset();
        mDepthAttachment.DepthStencilBuffer.Reset();
    }

    void DX11Framebuffer::Clear(const glm::vec4& clearColor) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        for (size_t i = 0; i < mColorAttachments.size(); i++)
            deviceContext->ClearRenderTargetView(mColorAttachments[i].RenderTargetView.Get(), (float*)&clearColor);

        if (mDepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
            deviceContext->ClearDepthStencilView(mDepthAttachment.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void DX11Framebuffer::Resize(Uint width, Uint height)
    {
        if (width == 0 || height == 0 || width > sMaxFramebufferSize || height > sMaxFramebufferSize)
        {
            ELECTRO_WARN("Attempted to resize framebuffer to %i, %i", width, height);
            return;
        }

        mSpecification.Width = width;
        mSpecification.Height = height;
        Invalidate();
    }
}

