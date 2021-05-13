//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"

namespace Electro
{
    struct SPIRVHandle;
    class ShaderReflectionData;
    enum class ShaderDomain
    {
        None = 0,
        Vertex,
        Pixel,
        Compute
    };

    class Shader : public Asset
    {
    public:
        virtual ~Shader() = default;
        virtual void Bind() const = 0;
        virtual void Reload() = 0;
        virtual const String GetSource(const ShaderDomain& domain = ShaderDomain::None) const = 0;
        virtual const SPIRVHandle GetSPIRV(const ShaderDomain& domain) const = 0;
        virtual const ShaderReflectionData GetReflectionData(const ShaderDomain& domain) const = 0;
    };
}
