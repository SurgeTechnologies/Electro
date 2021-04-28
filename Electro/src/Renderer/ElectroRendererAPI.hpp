//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Interface/ElectroPipeline.hpp"
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

    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL = 1,
            DX11 = 2,
        };
    public:
        static RenderAPICapabilities& GetCapabilities()
        {
            static RenderAPICapabilities capabilities;
            return capabilities;
        }

        virtual ~RendererAPI() = default;
        virtual void Init() = 0;
        virtual void SetViewport(Uint x, Uint y, Uint width, Uint height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;
        virtual void Draw(Uint count) = 0;
        virtual void DrawIndexed(Ref<Pipeline>& pipeline, Uint indexCount = 0) = 0;
        virtual void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex) = 0;
        virtual void BindBackbuffer() = 0;
        virtual void BeginWireframe() = 0;
        virtual void EndWireframe() = 0;
        virtual void SetDepthTest(DepthTestFunc type) = 0;
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
        static API GetAPI() { return sAPI; }
    private:
        static API sAPI;
    };

}