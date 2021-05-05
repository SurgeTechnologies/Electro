//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    //Currently only cares for Texture binding and its name in shaders
    //TODO: Extend this to support the Constant Buffers
    enum class ShaderDomain;
    struct ShaderResource
    {
        Uint Binding;
        String Name;
    };

    class ShaderReflectionData
    {
    public:
        const void SetDomain(const ShaderDomain& domain) { mShaderDomain = domain; }
        const void PushResource(const ShaderResource& res);
        const Uint GetRegister(const String& resourceName);
        Vector<ShaderResource>& GetResources() { return mShaderResources; }
    private:
        ShaderDomain mShaderDomain;
        Vector<ShaderResource> mShaderResources;
    };
}
