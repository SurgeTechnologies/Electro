//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class FramebufferTextureFormat
    {
        NONE = 0,

        // Color
        RGBA32F = 2,
        RGBA8UNORM = 28,
        R32SINT = 43,

        // Depth/Stencil
        R32VOID = 39,
        D24S8UINT = 45, // D24_UNORM_S8_UINT

        DEPTH = D24S8UINT,
        SHADOW = R32VOID
    };

    enum class FrameBufferFlags
    {
        DEFAULT = 0,
        COMPUTEWRITE
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format)
            : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::NONE;
        // TODO : Filtering/Wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments) {}

        Vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        Uint Width = 0, Height = 0;
        FramebufferAttachmentSpecification Attachments;
        FrameBufferFlags Flags;
        Uint Samples = 1;
        bool SwapChainTarget = false;
    };

    class Framebuffer : public IElectroRef
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Invalidate() = 0;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void PSBindColorBufferAsTexture(Uint index, Uint slot) const = 0;
        virtual void CSBindColorBufferAsTexture(Uint index, Uint slot) const = 0;

        virtual void PSUnbindColorBufferAsTexture(Uint slot) const = 0;
        virtual void CSUnbindColorBufferAsTexture(Uint slot) const = 0;

        virtual void BindDepthBufferAsTexture(Uint slot) const = 0;
        virtual void UnbindDepthBufferAsTexture(Uint slot) const = 0;

        virtual void CSBindUAV(Uint textureIndex, Uint slot) const = 0;
        virtual void CSUnbindUAV(Uint slot) const = 0;

        virtual void Resize(Uint width, Uint height) = 0;
        virtual void EnsureSize(Uint width, Uint height) = 0;
        virtual void* GetColorAttachmentID(Uint index = 0) const = 0;
        virtual void* GetDepthAttachmentID() const = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;
        virtual void Clear(const glm::vec4& clearColor = { 0.1f, 0.1f, 0.1f, 1.0f }) const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}
