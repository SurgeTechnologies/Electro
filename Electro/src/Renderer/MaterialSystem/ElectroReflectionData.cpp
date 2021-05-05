//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroReflectionData.hpp"
#include "Renderer/Interface/ElectroShader.hpp"

namespace Electro
{
    const void ShaderReflectionData::PushResource(const ShaderResource& res)
    {
        mShaderResources.push_back(res);
    }

    const Uint ShaderReflectionData::GetRegister(const String& resourceName)
    {
        for (const ShaderResource& resource : mShaderResources)
            if (resource.Name == resourceName)
                return resource.Binding;
    }
}
