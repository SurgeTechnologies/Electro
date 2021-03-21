//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroRendererAPI.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class DX11RendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;
        virtual void SetViewport(Uint x, Uint y, Uint width, Uint height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;
        virtual void DrawIndexed(Ref<Pipeline>& pipeline, Uint indexCount = 0) override;
        virtual void DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex) override;
        virtual void BindBackbuffer() override;
        virtual void BeginWireframe() override;
        virtual void EndWireframe() override;
        virtual void SetDepthTest(DepthTestFunc type) override;
    private:
        glm::vec4 mClearColor;
    };
}
