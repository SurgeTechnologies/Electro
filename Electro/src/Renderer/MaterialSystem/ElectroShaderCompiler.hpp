//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroRef.hpp"

namespace Electro
{
    enum class ShaderDomain;
    class Shader;
    class ShaderReflectionData;

    struct SPIRVHandle
    {
        SPIRVHandle() = default;
        SPIRVHandle(const Vector<Uint>& spirv, const String& fileName, const ShaderDomain& domain)
            : SPIRV(spirv), FileName(fileName), Domain(domain) {}

        Vector<Uint> SPIRV;
        String FileName;
        ShaderDomain Domain;
    };

    class ShaderCompiler
    {
    public:
        static E_NODISCARD SPIRVHandle CompileToSPIRv(const String& name, const String& shaderSource, const ShaderDomain& domain, const bool removeOld = false);
        static E_NODISCARD String CrossCompileToGLSL(const SPIRVHandle& spirv);
        static E_NODISCARD String CrossCompileToHLSL(const SPIRVHandle& spirv);
        static E_NODISCARD ShaderReflectionData Reflect(const SPIRVHandle& spirv, const String& shaderName);
    };
}
