//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Renderbuffer.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/DX11/DX11Renderbuffer.hpp"

namespace Electro
{
    Ref<Renderbuffer> Renderbuffer::Create(const FramebufferSpecification& spec)
    {
        switch (Renderer::GetBackend())
        {
            case RendererBackend::DirectX11:
                return Ref<DX11Renderbuffer>::Create(spec);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
