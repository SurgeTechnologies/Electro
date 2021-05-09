//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "RendererAPI.hpp"

namespace Electro
{
    class RenderCommand
    {
    public:
        static void SetViewport(Uint x, Uint y, Uint width, Uint height)
        {
            sRendererAPI->SetViewport(x, y, width, height);
        }

        static void SetClearColor(const glm::vec4& color)
        {
            sRendererAPI->SetClearColor(color);
        }
        static void Clear()
        {
            sRendererAPI->Clear();
        }

        static void Draw(Uint count)
        {
            sRendererAPI->Draw(count);
        }

        static void DrawIndexed(Ref<Pipeline>& pipeline, Uint count = 0)
        {
            sRendererAPI->DrawIndexed(pipeline, count);
        }

        static void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex)
        {
            sRendererAPI->DrawIndexedMesh(indexCount, baseIndex, baseVertex);
        }

        static void BindBackbuffer()
        {
            sRendererAPI->BindBackbuffer();
        }

        static void BeginWireframe()
        {
            sRendererAPI->BeginWireframe();
        }

        static void EndWireframe()
        {
            sRendererAPI->EndWireframe();
        }

        static void SetDepthTest(DepthTestFunc type)
        {
            sRendererAPI->SetDepthTest(type);
        }

        static void SetPrimitiveTopology(PrimitiveTopology topology)
        {
            sRendererAPI->SetPrimitiveTopology(topology);
        }
    private:
        static Scope<RendererAPI> sRendererAPI;
    };
}
