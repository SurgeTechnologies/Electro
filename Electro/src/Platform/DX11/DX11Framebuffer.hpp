//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Framebuffer.hpp"
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
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
    };

    class DX11Framebuffer : public Framebuffer
    {
    public:
        DX11Framebuffer(const FramebufferSpecification& spec);
        ~DX11Framebuffer() = default;

        virtual void Invalidate() override;
        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void PSBindColorBufferAsTexture(Uint index, Uint slot) const override;
        virtual void CSBindColorBufferAsTexture(Uint index, Uint slot) const override;

        virtual void PSUnbindColorBufferAsTexture(Uint slot) const override;
        virtual void CSUnbindColorBufferAsTexture(Uint slot) const override;

        virtual void BindDepthBufferAsTexture(Uint slot) const override;
        virtual void UnbindDepthBufferAsTexture(Uint slot) const override;

        virtual void CSBindUAV(Uint textureIndex, Uint slot) const override;
        virtual void CSUnbindUAV(Uint slot) const override;

        virtual void Resize(Uint width, Uint height) override;
        virtual void EnsureSize(Uint width, Uint height) override;
        virtual void* GetColorAttachmentID(Uint index = 0) const override { return mColorAttachments[index].ShaderResourceView.Get(); }
        virtual void* GetDepthAttachmentID() const override { return mDepthAttachment.ShaderResourceView.Get(); }

        virtual const FramebufferSpecification& GetSpecification() const override { return mSpecification; }
        virtual void Clear(const glm::vec4& clearColor = { 0.1f, 0.1f, 0.1f, 1.0f }) const override;
    private:
        void Clean();
    private:
        FramebufferSpecification mSpecification;

        Vector<FramebufferTextureSpecification> mColorAttachmentSpecifications;
        Vector<FramebufferColorAttachment> mColorAttachments;

        FramebufferDepthAttachment mDepthAttachment;
        FramebufferTextureSpecification mDepthAttachmentSpecification = FramebufferTextureFormat::None;

        ID3D11ShaderResourceView* mNullSRV = { nullptr };
        ID3D11UnorderedAccessView* mNullUAV = { nullptr };
        ID3D11RenderTargetView* mNullRTV = { nullptr };
        D3D11_VIEWPORT mViewport;
    };
}
