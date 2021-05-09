//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class FramebufferTextureFormat
    {
        None = 0,

        //Color
        RGBA32F = 2,
        R8G8B8A8_UNORM = 28,
        R32_SINT = 43,

        //Depth/Stencil
        D24_UNORM_S8_UINT = 45,

        //Default
        Depth = D24_UNORM_S8_UINT
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format)
            : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
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
        Uint Samples = 1;
        String Name = "None";
        bool SwapChainTarget = false;
    };

    class Framebuffer : public IElectroRef
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Invalidate() = 0;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual void Resize(Uint width, Uint height) = 0;
        virtual String GetName() = 0;
        virtual void* GetColorAttachmentID(Uint index = 0) const = 0;
        virtual void* GetDepthAttachmentID() const = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;
        virtual void Clear(const glm::vec4& clearColor) = 0;
    };
}
