//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11RendererAPI.hpp"
#include "Core/ElectroApplication.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    void DX11RendererAPI::Init()
    {
    }

    void DX11RendererAPI::SetViewport(Uint x, Uint y, Uint width, Uint height)
    {
        DX11Internal::Resize(width, height);
    }

    void DX11RendererAPI::SetClearColor(const glm::vec4& color)
    {
        mClearColor = color;
    }

    void DX11RendererAPI::Clear()
    {
        DX11Internal::GetBackbuffer()->Clear(mClearColor);
    }

    void DX11RendererAPI::DrawIndexed(Ref<Pipeline>& pipeline, Uint indexCount)
    {
        Uint count = indexCount ? indexCount : pipeline->GetSpecification().IndexBuffer->GetCount();
        DX11Internal::GetDeviceContext()->DrawIndexed(count, 0, 0);
    }

    void DX11RendererAPI::DrawIndexedMesh(Uint indexCount, Uint baseIndex, Uint baseVertex)
    {
        DX11Internal::GetDeviceContext()->DrawIndexed(indexCount, baseIndex, baseVertex);
    }

    void DX11RendererAPI::BindBackbuffer()
    {
        DX11Internal::BindBackbuffer();
    }

    void DX11RendererAPI::BeginWireframe()
    {
        DX11Internal::BeginWireframe();
    }

    void DX11RendererAPI::EndWireframe()
    {
        DX11Internal::EndWireframe();
    }

    void DX11RendererAPI::SetDepthTest(DepthTestFunc type)
    {
        DX11Internal::GetDeviceContext()->OMSetDepthStencilState(DX11Internal::GetDepthStencilState(type), 1);
    }
}