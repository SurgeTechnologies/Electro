//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "VertexBuffer.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Platform/DX11/DX11VertexBuffer.hpp"

namespace Electro
{
    Ref<VertexBuffer> VertexBuffer::Create(Uint size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11VertexBuffer>::Create(size);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void* vertices, Uint size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11VertexBuffer>::Create(vertices, size);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
