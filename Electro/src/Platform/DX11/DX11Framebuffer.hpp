//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ElectroFramebuffer.hpp"
#include <d3d11.h>
#include <wrl.h>

namespace Electro
{
    struct FramebufferColorAttachment
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTargetTexture;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
    };

    struct FramebufferDepthAttachment
    {
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilBuffer;
    };

    class DX11Framebuffer : public Framebuffer
    {
    public:
        DX11Framebuffer(const FramebufferSpecification& spec);
        ~DX11Framebuffer() = default;

        virtual void Invalidate() override;
        virtual void Bind() const override;
        virtual void Unbind() const override {};
        virtual void Resize(Uint width, Uint height) override;
        virtual String GetName() override { return mSpecification.Name; }
        virtual void* GetColorAttachmentID(Uint index = 0) const override { return (void*)mColorAttachments[index].ShaderResourceView.Get(); }
        virtual void* GetDepthAttachmentID() const override { return (void*)mDepthAttachment.DepthStencilView.Get(); }

        virtual const FramebufferSpecification& GetSpecification() const override { return mSpecification; }
        virtual void Clear(const glm::vec4& clearColor) override;
    private:
        void Clean();
    private:
        FramebufferSpecification mSpecification;

        Vector<FramebufferTextureSpecification> mColorAttachmentSpecifications;
        Vector<FramebufferColorAttachment> mColorAttachments;

        FramebufferDepthAttachment mDepthAttachment;
        FramebufferTextureSpecification mDepthAttachmentSpecification = FramebufferTextureFormat::None;
        D3D11_VIEWPORT mViewport;
    };
}
