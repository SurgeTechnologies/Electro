//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Renderbuffer.hpp"
#include <d3d11.h>
#include <wrl.h>

namespace Electro
{
    struct FramebufferColorAttachment
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTargetTexture;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;
    };

    struct FramebufferDepthAttachment
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
    };

    class DX11Renderbuffer : public Renderbuffer
    {
    public:
        DX11Renderbuffer(const RenderbufferSpecification& spec);
        ~DX11Renderbuffer() = default;

        virtual void Invalidate() override;
        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void BindColorBuffer(Uint index, Uint slot, ShaderDomain shaderDomain) const override;
        virtual void BindDepthBuffer(Uint slot, ShaderDomain shaderDomain) const override;
        virtual void UnbindBuffer(Uint slot, ShaderDomain shaderDomain) const override;

        virtual void CSBindUAV(Uint textureIndex, Uint slot) const override;
        virtual void CSUnbindUAV(Uint slot) const override;

        virtual void Resize(Uint width, Uint height) override;
        virtual void EnsureSize(Uint width, Uint height) override;
        virtual void* GetColorAttachmentID(Uint index = 0) const override { return mColorAttachments[index].ShaderResourceView.Get(); }
        virtual void* GetDepthAttachmentID() const override { return mDepthAttachment.ShaderResourceView.Get(); }

        virtual const RenderbufferSpecification& GetSpecification() const override { return mSpecification; }
        virtual void Clear(const glm::vec4& clearColor = { 0.0f, 0.0f, 0.0f, 0.0f }) const override;
    private:
        void Clean();
    private:
        RenderbufferSpecification mSpecification;

        Vector<RenderBufferTextureSpecification> mColorAttachmentSpecifications;
        Vector<FramebufferColorAttachment> mColorAttachments;

        FramebufferDepthAttachment mDepthAttachment;
        RenderBufferTextureSpecification mDepthAttachmentSpecification = RenderBufferTextureFormat::NONE;

        ID3D11ShaderResourceView* mNullSRV = { nullptr };
        ID3D11UnorderedAccessView* mNullUAV = { nullptr };
        ID3D11RenderTargetView* mNullRTV = { nullptr };
        D3D11_VIEWPORT mViewport;
    };
}
