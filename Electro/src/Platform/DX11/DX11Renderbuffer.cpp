//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Renderbuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    // Max: 8K Texture; 8192 / 1204 = 8
    static const Uint sMaxFramebufferSize = 8192;

    namespace Utils
    {
        static void AttachColorTexture(RenderBufferTextureSpecification textureSpec, RenderbufferSpecification renderbufferSpec, RenderbufferColorAttachment* outColorAttachment)
        {
            ID3D11Device* device = DX11Internal::GetDevice();

            // Render Target texture
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = renderbufferSpec.Width;
            textureDesc.Height = renderbufferSpec.Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            if (renderbufferSpec.Flags == RenderBufferFlags::COMPUTEWRITE)
                textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

            DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &outColorAttachment->RenderTargetTexture));

#ifdef E_DEBUG
            if (!renderbufferSpec.DebugName.empty())
                outColorAttachment->RenderTargetTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(renderbufferSpec.DebugName.c_str()), renderbufferSpec.DebugName.c_str());
#endif
            // Render Target View
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;
            DX_CALL(device->CreateRenderTargetView(outColorAttachment->RenderTargetTexture.Get(), &renderTargetViewDesc, &outColorAttachment->RenderTargetView));

            {
                // Shader Resource View
                D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
                shaderResourceViewDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
                shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
                shaderResourceViewDesc.Texture2D.MipLevels = 1;
                DX_CALL(device->CreateShaderResourceView(outColorAttachment->RenderTargetTexture.Get(), &shaderResourceViewDesc, &outColorAttachment->ShaderResourceView));
            }

            if (renderbufferSpec.Flags == RenderBufferFlags::COMPUTEWRITE)
            {
                // Shader Resource View
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = 0;
                DX_CALL(device->CreateUnorderedAccessView(outColorAttachment->RenderTargetTexture.Get(), &uavDesc, &outColorAttachment->UnorderedAccessView));
            }
        }

        static void AttachDepthTexture(RenderBufferTextureSpecification textureSpec, RenderbufferSpecification renderbufferSpec, RenderbufferDepthAttachment* outDepthAttachment)
        {
            ID3D11Device* device = DX11Internal::GetDevice();

            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = renderbufferSpec.Width;
            textureDesc.Height = renderbufferSpec.Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = static_cast<DXGI_FORMAT>(textureSpec.TextureFormat);
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            if (static_cast<DXGI_FORMAT>(textureSpec.TextureFormat) == DXGI_FORMAT_R32_TYPELESS)
                textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

            DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &outDepthAttachment->DepthStencilBuffer));

#ifdef E_DEBUG
            if (!renderbufferSpec.DebugName.empty())
                outDepthAttachment->DepthStencilBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(renderbufferSpec.DebugName.c_str()), renderbufferSpec.DebugName.c_str());
#endif

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

        static void AttachToSwapchain(RenderbufferColorAttachment* outColorAttachment, RenderbufferSpecification renderbufferSpec)
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
            ID3D11Device* device = DX11Internal::GetDevice();
            IDXGISwapChain* swapChain = DX11Internal::GetSwapChain();

            swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
            device->CreateRenderTargetView(backBuffer.Get(), nullptr, &outColorAttachment->RenderTargetView);
#ifdef E_DEBUG
            if (!renderbufferSpec.DebugName.empty())
                outColorAttachment->RenderTargetTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(renderbufferSpec.DebugName.c_str()), renderbufferSpec.DebugName.c_str());
#endif
        }

        static bool IsDepthFormat(RenderBufferTextureFormat format)
        {
            switch (format)
            {
                case RenderBufferTextureFormat::D24S8UINT: return true;
                case RenderBufferTextureFormat::R32VOID: return true;
            }

            return false;
        }
    }

    DX11Renderbuffer::DX11Renderbuffer(const RenderbufferSpecification& spec)
        : mSpecification(spec)
    {
        for (const RenderBufferTextureSpecification& format : mSpecification.Attachments.Attachments)
        {
            if (!Utils::IsDepthFormat(format.TextureFormat))
                mColorAttachmentSpecifications.emplace_back(format);
            else
                mDepthAttachmentSpecification = format;
        }

        Invalidate();
    }

    void DX11Renderbuffer::Bind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->RSSetViewports(1, &mViewport);

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderViews[2];
        for (Uint i = 0; i < mColorAttachments.size(); i++)
            pRenderViews[i] = mColorAttachments[i].RenderTargetView;

        deviceContext->OMSetRenderTargets(static_cast<Uint>(mColorAttachments.size()), pRenderViews[0].GetAddressOf(), mDepthAttachment.DepthStencilView.Get());
    }

    void DX11Renderbuffer::Unbind() const
    {
        DX11Internal::GetDeviceContext()->OMSetRenderTargets(1, &mNullRTV, NULL);
    }

    void DX11Renderbuffer::BindColorBuffer(Uint index, Uint slot, ShaderDomain shaderDomain) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        switch (shaderDomain)
        {
            case ShaderDomain::PIXEL:   deviceContext->PSSetShaderResources(slot, 1, mColorAttachments[index].ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::COMPUTE: deviceContext->CSSetShaderResources(slot, 1, mColorAttachments[index].ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::VERTEX:  deviceContext->VSSetShaderResources(slot, 1, mColorAttachments[index].ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::NONE:    E_INTERNAL_ASSERT("ShaderDomain::NONE is invalid in this context!") break;
        }
    }

    void DX11Renderbuffer::BindDepthBuffer(Uint slot, ShaderDomain shaderDomain) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        switch (shaderDomain)
        {
            case ShaderDomain::PIXEL:   deviceContext->PSSetShaderResources(slot, 1, mDepthAttachment.ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::COMPUTE: deviceContext->CSSetShaderResources(slot, 1, mDepthAttachment.ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::VERTEX:  deviceContext->VSSetShaderResources(slot, 1, mDepthAttachment.ShaderResourceView.GetAddressOf()); break;
            case ShaderDomain::NONE:    E_INTERNAL_ASSERT("ShaderDomain::NONE is invalid in this context!") break;
        }
    }

    void DX11Renderbuffer::UnbindBuffer(Uint slot, ShaderDomain shaderDomain) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        switch (shaderDomain)
        {
            case ShaderDomain::PIXEL:   deviceContext->PSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::COMPUTE: deviceContext->CSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::VERTEX:  deviceContext->VSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::NONE:    E_INTERNAL_ASSERT("ShaderDomain::NONE is invalid in this context!") break;
        }
    }

    void DX11Renderbuffer::CSBindUAV(Uint textureIndex, Uint slot) const
    {
        constexpr UINT noOffset = -1;
        DX11Internal::GetDeviceContext()->CSSetUnorderedAccessViews(slot, 1, mColorAttachments[textureIndex].UnorderedAccessView.GetAddressOf(), &noOffset);
    }

    void DX11Renderbuffer::CSUnbindUAV(Uint slot) const
    {
        constexpr UINT noOffset = -1;
        DX11Internal::GetDeviceContext()->CSSetUnorderedAccessViews(slot, 1, &mNullUAV, &noOffset);
    }

    void DX11Renderbuffer::Invalidate()
    {
        Clean();
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(mSpecification.Width);
        mViewport.Height = static_cast<float>(mSpecification.Height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        if (!mColorAttachmentSpecifications.empty())
        {
            mColorAttachments.resize(mColorAttachmentSpecifications.size());

            // Attachments
            for (size_t i = 0; i < mColorAttachments.size(); i++)
            {
                if (mSpecification.SwapChainTarget)
                {
                    Utils::AttachToSwapchain(&mColorAttachments[i], mSpecification);
                    break;
                }

                switch (mColorAttachmentSpecifications[i].TextureFormat)
                {
                    case RenderBufferTextureFormat::RGBA32F:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                    case RenderBufferTextureFormat::RGBA8UNORM:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                    case RenderBufferTextureFormat::R32SINT:
                        Utils::AttachColorTexture(mColorAttachmentSpecifications[i], mSpecification, &mColorAttachments[i]); break;
                }
            }
        }

        if (mDepthAttachmentSpecification.TextureFormat != RenderBufferTextureFormat::NONE)
        {
            switch (mDepthAttachmentSpecification.TextureFormat)
            {
                case RenderBufferTextureFormat::D24S8UINT:
                    Utils::AttachDepthTexture(mDepthAttachmentSpecification, mSpecification, &mDepthAttachment); break;
                case RenderBufferTextureFormat::R32VOID:
                    Utils::AttachDepthTexture(mDepthAttachmentSpecification, mSpecification, &mDepthAttachment); break;
            }
        }
    }

    void DX11Renderbuffer::Clean()
    {
        for (size_t i = 0; i < mColorAttachments.size(); i++)
        {
            mColorAttachments[i].RenderTargetTexture.Reset();
            mColorAttachments[i].RenderTargetView.Reset();
            mColorAttachments[i].ShaderResourceView.Reset();
        }

        mColorAttachments.clear();

        mDepthAttachment.DepthStencilView.Reset();
        mDepthAttachment.DepthStencilBuffer.Reset();
    }

    void DX11Renderbuffer::Clear(const glm::vec4& clearColor) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();

        for (Uint i = 0; i < mColorAttachments.size(); i++)
            deviceContext->ClearRenderTargetView(mColorAttachments[i].RenderTargetView.Get(), (float*)&clearColor);

        if (mDepthAttachmentSpecification.TextureFormat != RenderBufferTextureFormat::NONE)
            deviceContext->ClearDepthStencilView(mDepthAttachment.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void DX11Renderbuffer::Resize(Uint width, Uint height)
    {
        if (width == 0 || height == 0 || width > sMaxFramebufferSize || height > sMaxFramebufferSize)
        {
            Log::Warn("Attempted to resize renderbuffer to {0}, {0}", width, height);
            return;
        }

        mSpecification.Width = width;
        mSpecification.Height = height;
        Invalidate();
    }

    void DX11Renderbuffer::EnsureSize(Uint width, Uint height)
    {
        if (width != mSpecification.Width || height != mSpecification.Height)
        {
            mSpecification.Width = width;
            mSpecification.Height = height;
            Invalidate();
        }
    }

}

