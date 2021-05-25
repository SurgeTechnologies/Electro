//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/Pipeline.hpp"
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
        Undefined = 0,
        Pointlist = 1,
        Linelist = 2,
        Linestrip = 3,
        Trianglelist = 4,
        Trianglestrip = 5
    };

    enum class DepthTestFunc
    {
        Never = 0,
        Less = 1,
        Equal = 2,
        LEqual = 3,
        Greater = 4,
        NotEqual = 5,
        GEqual = 6,
        Always = 7
    };

    enum class CullMode
    {
        Front,
        Back,
        None
    };

    class RendererAPI
    {
    public:
        enum class API
        {
            DX11 = 2,
            OpenGL = 0,
        };
    public:
        static RenderAPICapabilities& GetCapabilities()
        {
            static RenderAPICapabilities capabilities;
            return capabilities;
        }

        virtual ~RendererAPI() = default;
        virtual void SetViewport(Uint width, Uint height) = 0;
        virtual void ResizeBackbuffer(Uint x, Uint y, Uint width, Uint height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void Draw(Uint count) = 0;
        virtual void DrawIndexed(Ref<Pipeline>& pipeline, Uint indexCount = 0) = 0;
        virtual void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex) = 0;
        virtual void BindBackbuffer() = 0;
        virtual void BeginWireframe() = 0;
        virtual void EndWireframe() = 0;
        virtual void SetDepthTest(DepthTestFunc type) = 0;
        virtual void SetCullMode(CullMode cullMode) = 0;
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
        static API GetAPI() { return sAPI; }
    private:
        static API sAPI;
    };

}
