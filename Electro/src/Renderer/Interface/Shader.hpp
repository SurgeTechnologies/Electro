//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"

namespace Electro
{
    struct SPIRVHandle;
    class ShaderReflectionData;
    enum class ShaderDomain
    {
        NONE = 0,
        VERTEX,
        PIXEL,
        COMPUTE
    };

    class Shader : public IElectroRef
    {
    public:
        virtual void Bind() const = 0;
        virtual void Reload() = 0;
        virtual const String& GetSource(const ShaderDomain& domain = ShaderDomain::NONE) const = 0;
        virtual const SPIRVHandle& GetSPIRV(const ShaderDomain& domain) const = 0;
        virtual const ShaderReflectionData& GetReflectionData(const ShaderDomain& domain) const = 0;
        virtual const String GetName() const = 0;
        virtual const String GetPath() const = 0;

        static Ref<Shader> Create(const String& filepath);
    };
}
