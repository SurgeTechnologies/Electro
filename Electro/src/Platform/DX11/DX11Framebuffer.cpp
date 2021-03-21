//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Framebuffer.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    static const Uint sMaxFramebufferSize = 8192;
    DXGI_FORMAT SpikeFormatToDX11Format(FormatCode code)
    {
        switch (code)
        {
            case FormatCode::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case FormatCode::R8G8B8A8_UNORM:     return DXGI_FORMAT_R8G8B8A8_UNORM;
            case FormatCode::D24_UNORM_S8_UINT:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
        }
        ELECTRO_ERROR("No correct DX11 format found for the given Spike Format. Returning DXGI_FORMAT_R32G32B32A32_FLOAT...");
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }

    DX11Framebuffer::DX11Framebuffer(const FramebufferSpecification& spec)
        :m_Specification(spec)
    {
        Invalidate();
    }

    void DX11Framebuffer::Invalidate()
    {
        Clean();
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(m_Specification.Width);
        mViewport.Height = static_cast<float>(m_Specification.Height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        for (auto desc : m_Specification.BufferDescriptions)
        {
            if (IsDepthFormat(desc.Format))
            {
                m_IsDepth = true;
                CreateDepthView(desc);
            }
            else
            {
                if (m_Specification.SwapChainTarget)
                    CreateSwapChainView();
                else
                    CreateColorView(desc);
            }
        }
    }

    void DX11Framebuffer::Clean()
    {
        m_RenderTargetTexture.Reset();
        m_RenderTargetView.Reset();
        m_SRV.Reset();

        m_DepthStencilView.Reset();
        m_DepthStencilState.Reset();
        m_DepthStencilBuffer.Reset();
    }

    void DX11Framebuffer::Bind()
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->RSSetViewports(1, &mViewport);
        deviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

        if (m_IsDepth)
            deviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
    }

    void DX11Framebuffer::Unbind() {}

    void DX11Framebuffer::Resize(Uint width, Uint height)
    {
        if (width == 0 || height == 0 || width > sMaxFramebufferSize || height > sMaxFramebufferSize)
        {
            ELECTRO_WARN("Attempted to resize framebuffer to %i, %i", width, height);
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

    void DX11Framebuffer::Clear(const glm::vec4& clearColor)
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        deviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), (float*)&clearColor);
        if (m_IsDepth)
            deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    RendererID DX11Framebuffer::GetSwapChainTarget() { return (RendererID)m_RenderTargetView.Get(); }
    void DX11Framebuffer::CreateSwapChainView()
    {
        ID3D11Texture2D* backBuffer;
        DX11Internal::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer);
        DX11Internal::GetDevice()->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
        backBuffer->Release();
    }

    void DX11Framebuffer::CreateColorView(FramebufferSpecification::BufferDesc desc)
    {
        ID3D11Device* device = DX11Internal::GetDevice();

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = m_Specification.Width;
        textureDesc.Height = m_Specification.Height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = SpikeFormatToDX11Format(desc.Format);
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = (D3D11_BIND_FLAG)desc.BindFlags;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &m_RenderTargetTexture));
        //https://renderdoc.org/docs/how/how_view_texture.html
        m_RenderTargetTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Render Target(Not a SwapChain)"), "Render Target(Not a SwapChain)");

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
        renderTargetViewDesc.Format = SpikeFormatToDX11Format(desc.Format);
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;
        DX_CALL(device->CreateRenderTargetView(m_RenderTargetTexture.Get(), &renderTargetViewDesc, &m_RenderTargetView));

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
        shaderResourceViewDesc.Format = SpikeFormatToDX11Format(desc.Format);
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;
        DX_CALL(device->CreateShaderResourceView(m_RenderTargetTexture.Get(), &shaderResourceViewDesc, &m_SRV));
    }

    void DX11Framebuffer::CreateDepthView(FramebufferSpecification::BufferDesc desc)
    {
        ID3D11Device* device = DX11Internal::GetDevice();

        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = m_Specification.Width;
        descDepth.Height = m_Specification.Height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = SpikeFormatToDX11Format(desc.Format);
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        DX_CALL(device->CreateTexture2D(&descDepth, nullptr, &m_DepthStencilBuffer));

        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = true;
        dsDesc.StencilReadMask = 0xFF;
        dsDesc.StencilWriteMask = 0xFF;
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        DX_CALL(device->CreateDepthStencilState(&dsDesc, &m_DepthStencilState));

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format = SpikeFormatToDX11Format(desc.Format);
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;
        DX_CALL(device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, &m_DepthStencilView));
    }

    bool DX11Framebuffer::IsDepthFormat(const FormatCode format)
    {
        if (format == FormatCode::D24_UNORM_S8_UINT)
            return true;
        else
            return false;
    }

}