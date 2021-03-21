//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroPipeline.hpp"
#include "ElectroRenderer.hpp"
#include "Platform/DX11/DX11Pipeline.hpp"

namespace Electro
{
    Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11Pipeline>::Create(spec);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

}