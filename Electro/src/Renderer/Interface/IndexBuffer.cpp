//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "IndexBuffer.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/DX11/DX11IndexBuffer.hpp"

namespace Electro
{
    Ref<IndexBuffer> IndexBuffer::Create(void* indices, Uint count)
    {
        switch (Renderer::GetBackend())
        {
            case RendererBackend::DirectX11:
                return Ref<DX11IndexBuffer>::Create(indices, count);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
