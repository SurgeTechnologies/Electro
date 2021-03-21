//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class FormatCode
    {
        R32G32B32A32_FLOAT = 2,
        R8G8B8A8_UNORM = 28,
        D24_UNORM_S8_UINT = 45
    };

    enum class BindFlag
    {
        VERTEX_BUFFER = 0x1L,
        INDEX_BUFFER = 0x2L,
        CONSTANT_BUFFER = 0x4L,
        SHADER_RESOURCE = 0x8L,
        STREAM_OUTPUT = 0x10L,
        RENDER_TARGET = 0x20L,
        DEPTH_STENCIL = 0x40L,
        UNORDERED_ACCESS = 0x80L,
        DECODER = 0x200L,
        VIDEO_ENCODER = 0x400L
    };
    inline BindFlag operator|(BindFlag a, BindFlag b) { return (BindFlag)((Uint)a | (Uint)b); };

    struct FramebufferSpecification
    {
        Uint Width = 0, Height = 0;
        Uint Samples = 1;

        struct BufferDesc
        {
            BufferDesc() = default;
            BufferDesc(FormatCode format, BindFlag bindFlags)
                :BindFlags(bindFlags), Format(format) {}

            FormatCode Format;
            BindFlag BindFlags;
        };

        Vector<BufferDesc> BufferDescriptions;
        bool SwapChainTarget = false;
    };

    class Framebuffer : public IElectroRef
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void Resize(Uint width, Uint height) = 0;
        virtual FramebufferSpecification& GetSpecification() = 0;
        virtual void Clear(const glm::vec4& clearColor) = 0;
        virtual RendererID GetColorViewID() = 0;
        virtual RendererID GetSwapChainTarget() = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}