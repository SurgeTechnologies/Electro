//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroRendererAPI.hpp"
#include "ElectroRenderer.hpp"
#include "ElectroVertexBuffer.hpp"
#include "Platform/DX11/DX11VertexBuffer.hpp"

namespace Electro
{
    Ref<VertexBuffer> VertexBuffer::Create(Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11VertexBuffer>::Create(size, layout);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }


    Ref<VertexBuffer> VertexBuffer::Create(void* vertices, Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:    return Ref<DX11VertexBuffer>::Create(vertices, size, layout);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}