//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "RendererAPI.hpp"

namespace Electro
{
    class RenderCommand
    {
    public:
        static void SetViewport(Viewport viewport)
        {
            sRendererAPI->SetViewport(viewport);
        }

        static Viewport GetViewport()
        {
            return sRendererAPI->GetViewport();
        }

        static void ResizeBackbuffer(const Uint x, const Uint y, const Uint width, const Uint height)
        {
            sRendererAPI->ResizeBackbuffer(x, y, width, height);
        }

        static void SetClearColor(const glm::vec4& color)
        {
            sRendererAPI->SetClearColor(color);
        }
        static void Clear()
        {
            sRendererAPI->Clear();
        }

        static void Draw(const Uint count)
        {
            sRendererAPI->Draw(count);
        }

        static void DrawIndexed(const Uint count)
        {
            sRendererAPI->DrawIndexed(count);
        }

        static void DrawIndexedMesh(const Uint indexCount, const Uint baseIndex, const Uint baseVertex)
        {
            sRendererAPI->DrawIndexedMesh(indexCount, baseIndex, baseVertex);
        }

        static Ref<Renderbuffer>& GetBackBuffer()
        {
            return sRendererAPI->GetBackBuffer();
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

        static void SetCullMode(const CullMode cullMode)
        {
            sRendererAPI->SetCullMode(cullMode);
        }

        static void EnableDepth()
        {
            sRendererAPI->EnableDepth();
        }

        static void DisableDepth()
        {
            sRendererAPI->DisableDepth();
        }

        static void SetPrimitiveTopology(PrimitiveTopology topology)
        {
            sRendererAPI->SetPrimitiveTopology(topology);
        }

        static void EnableAdditiveBlending()
        {
            sRendererAPI->EnableAdditiveBlending();
        }

        static void DisableAdditiveBlending()
        {
            sRendererAPI->DisableAdditiveBlending();
        }

        static void DispatchCompute(Uint threadGroupCountX, Uint threadGroupCountY, Uint threadGroupCountZ)
        {
            sRendererAPI->DispatchCompute(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
        }
    private:
        static Scope<RendererAPI> sRendererAPI;
    };
}
