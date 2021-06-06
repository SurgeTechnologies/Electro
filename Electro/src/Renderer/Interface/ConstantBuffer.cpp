//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ConstantBuffer.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Platform/DX11/DX11ConstantBuffer.hpp"

namespace Electro
{
    Ref<ConstantBuffer> ConstantBuffer::Create(Uint size, Uint bindSlot, DataUsage usage)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11ConstantBuffer>::Create(size, bindSlot, usage);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
