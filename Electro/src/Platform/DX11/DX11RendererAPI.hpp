//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/RendererAPI.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class DX11RendererAPI : public RendererAPI
    {
    public:
        virtual Viewport GetViewport() override;
        virtual void SetViewport(Viewport viewport) override;
        virtual void ResizeBackbuffer(Uint x, Uint y, Uint width, Uint height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;
        virtual void Draw(Uint count) override;
        virtual void DrawIndexed(Uint indexCount) override;
        virtual void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex) override;
        virtual Ref<Renderbuffer>& GetBackBuffer() const override;
        virtual void BindBackbuffer() override;
        virtual void BeginWireframe() override;
        virtual void EndWireframe() override;
        virtual void SetCullMode(CullMode cullMode) override;
        virtual void EnableDepth() override;
        virtual void DisableDepth() override;
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;
        virtual void EnableAdditiveBlending() override;
        virtual void DisableAdditiveBlending() override;
        virtual void DispatchCompute(Uint threadGroupCountX, Uint threadGroupCountY, Uint threadGroupCountZ) override;
    private:
        glm::vec4 mClearColor;
    };
}
