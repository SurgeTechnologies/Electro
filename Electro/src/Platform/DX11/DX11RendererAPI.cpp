//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11RendererAPI.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    Viewport DX11RendererAPI::GetViewport()
    {
        return DX11Internal::GetViewport();
    }

    void DX11RendererAPI::SetViewport(Viewport viewport)
    {
        DX11Internal::SetViewport(viewport);
    }

    void DX11RendererAPI::ResizeBackbuffer(Uint x, Uint y, Uint width, Uint height)
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

    void DX11RendererAPI::Draw(Uint count)
    {
        DX11Internal::GetDeviceContext()->Draw(count, 0);
    }

    void DX11RendererAPI::DrawIndexed(Uint indexCount)
    {
        DX11Internal::GetDeviceContext()->DrawIndexed(indexCount, 0, 0);
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

    void DX11RendererAPI::SetCullMode(CullMode cullMode)
    {
        DX11Internal::SetCullMode(cullMode);
    }

    void DX11RendererAPI::EnableDepth()
    {
        DX11Internal::EnableDepth();
    }

    void DX11RendererAPI::DisableDepth()
    {
        DX11Internal::DisableDepth();
    }

    static D3D_PRIMITIVE_TOPOLOGY ElectroTopologyToDX11Topology(PrimitiveTopology topology)
    {
        switch (topology)
        {
            case PrimitiveTopology::Trianglelist:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::Trianglestrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveTopology::Linelist:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopology::Pointlist:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopology::Linestrip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopology::Undefined:     return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
        return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    void DX11RendererAPI::SetPrimitiveTopology(PrimitiveTopology topology)
    {
        DX11Internal::GetDeviceContext()->IASetPrimitiveTopology(ElectroTopologyToDX11Topology(topology));
    }

    void DX11RendererAPI::EnableAdditiveBlending()
    {
        DX11Internal::EnableAdditiveBlending();
    }

    void DX11RendererAPI::DisableAdditiveBlending()
    {
        DX11Internal::DisableAdditiveBlending();
    }

    void DX11RendererAPI::DispatchCompute(Uint threadGroupCountX, Uint threadGroupCountY, Uint threadGroupCountZ)
    {
        DX11Internal::GetDeviceContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    Ref<Framebuffer>& DX11RendererAPI::GetBackBuffer() const
    {
        return DX11Internal::GetBackbuffer();
    }
}
