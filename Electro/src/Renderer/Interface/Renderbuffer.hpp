//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Renderer/Interface/Shader.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class RenderBufferTextureFormat
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

    enum class RenderBufferFlags
    {
        DEFAULT = 0,
        COMPUTEWRITE
    };

    struct RenderBufferTextureSpecification
    {
        RenderBufferTextureSpecification() = default;
        RenderBufferTextureSpecification(RenderBufferTextureFormat format)
            : TextureFormat(format) {}

        RenderBufferTextureFormat TextureFormat = RenderBufferTextureFormat::NONE;
    };

    struct RenderBufferAttachmentSpecification
    {
        RenderBufferAttachmentSpecification() = default;
        RenderBufferAttachmentSpecification(std::initializer_list<RenderBufferTextureSpecification> attachments)
            : Attachments(attachments) {}

        Vector<RenderBufferTextureSpecification> Attachments;
    };

    struct RenderbufferSpecification
    {
        Uint Width = 0, Height = 0;
        RenderBufferAttachmentSpecification Attachments;
        RenderBufferFlags Flags;
        Uint Samples = 1;
        bool SwapChainTarget = false;
    };

    class Renderbuffer : public IElectroRef
    {
    public:
        virtual ~Renderbuffer() = default;

        // Recreates all of the Buffers
        virtual void Invalidate() = 0;

        // Binds the RenderBuffer to the pipeline
        virtual void Bind() const = 0;

        // Unbinds the RenderBuffer from the pipeline
        virtual void Unbind() const = 0;

        // Binds the Color attachment at given index as Texture to given slot
        virtual void BindColorBuffer(Uint index, Uint slot, ShaderDomain shaderDomain) const = 0;

        // Binds the Depth attachment as Texture(Shader Resource) to given slot
        virtual void BindDepthBuffer(Uint slot, ShaderDomain shaderDomain) const = 0;

        // Unbinds the buffer(depth or color) as Texture from the given shaderDomain at the given slot
        virtual void UnbindBuffer(Uint slot, ShaderDomain shaderDomain) const = 0;

        // TODO: Move to Compute (GPGPU - General Purpose GPU) class
        virtual void CSBindUAV(Uint textureIndex, Uint slot) const = 0;
        virtual void CSUnbindUAV(Uint slot) const = 0;

        virtual void Resize(Uint width, Uint height) = 0;
        virtual void EnsureSize(Uint width, Uint height) = 0;
        virtual void* GetColorAttachmentID(Uint index = 0) const = 0;
        virtual void* GetDepthAttachmentID() const = 0;
        virtual const RenderbufferSpecification& GetSpecification() const = 0;
        virtual void Clear(const glm::vec4& clearColor = { 0.0f, 0.0f, 0.0f, 0.0f }) const = 0;

        static Ref<Renderbuffer> Create(const RenderbufferSpecification& spec);
    };
}
