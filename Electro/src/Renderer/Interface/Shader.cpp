//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Shader.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/DX11/DX11Shader.hpp"

namespace Electro
{
    Ref<Shader> Shader::Create(const String& filepath)
    {
        switch (Renderer::GetBackend())
        {
            case RendererBackend::DirectX11:
                return Ref<DX11Shader>::Create(filepath);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
