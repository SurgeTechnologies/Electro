//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Pipeline.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Platform/DX11/DX11Pipeline.hpp"

namespace Electro
{
    Ref<Pipeline> Pipeline::Create()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11Pipeline>::Create();
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
