//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/Pipeline.hpp"
#include "Renderer/Interface/Renderbuffer.hpp"
#include <glm\glm.hpp>
#include <memory>

namespace Electro
{
    struct RenderAPICapabilities
    {
        String Vendor;
        String Renderer;
        String Version;

        float MaxAnisotropy = 0.0f;
        int MaxTextureUnits = 0;
        int MaxSamples = 0;
    };

    enum class PrimitiveTopology
    {
        UNDEFINED = 0,
        POINTLIST = 1,
        LINELIST = 2,
        LINESTRIP = 3,
        TRIANGLELIST = 4,
        TRIANGLESTRIP = 5
    };

    enum class CullMode
    {
        FRONT,
        BACK,
        NONE
    };

    struct Viewport
    {
        Viewport() = default;
        Viewport(Uint width, Uint height)
            : Width(width), Height(height) {}

        Viewport(Uint width, Uint height, float minDepth, float maxDepth)
            : Width(width), Height(height), MinDepth(minDepth), MaxDepth(maxDepth) {}

        float TopLeftX = 0.0f;
        float TopLeftY = 0.0f;
        Uint Width = 0;
        Uint Height = 0;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0f;
    };

    class RendererAPI
    {
    public:
        static RenderAPICapabilities& GetCapabilities()
        {
            static RenderAPICapabilities capabilities;
            return capabilities;
        }

        virtual ~RendererAPI() = default;
        virtual Viewport GetViewport() = 0;
        virtual void SetViewport(Viewport viewport) = 0;
        virtual void ResizeBackbuffer(Uint x, Uint y, Uint width, Uint height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void Draw(Uint count) = 0;
        virtual void DrawIndexed(Uint indexCount) = 0;
        virtual void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex) = 0;
        virtual Ref<Renderbuffer>& GetBackBuffer() const = 0;
        virtual void BindBackbuffer() = 0;
        virtual void BeginWireframe() = 0;
        virtual void EndWireframe() = 0;
        virtual void SetCullMode(CullMode cullMode) = 0;
        virtual void EnableDepth() = 0;
        virtual void DisableDepth() = 0;
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
        virtual void EnableAdditiveBlending() = 0;
        virtual void DisableAdditiveBlending() = 0;
        virtual void DispatchCompute(Uint threadGroupCountX, Uint threadGroupCountY, Uint threadGroupCountZ) = 0;
    };

}
