//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroConstantBuffer.hpp"
#include "ElectroRendererAPI.hpp"
#include "Platform/DX11/DX11ConstantBuffer.hpp"

namespace Electro
{
    Ref<ConstantBuffer> ConstantBuffer::Create(const Ref<Shader>& shader, const String& name, void* data, const Uint size, const Uint bindSlot, ShaderDomain shaderDomain, DataUsage usage)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:    return Ref<DX11ConstantBuffer>::Create(shader, name, data, size, bindSlot, shaderDomain, usage);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
