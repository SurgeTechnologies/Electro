//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    enum class ShaderDomain;

    struct SPIRVHandle
    {
        SPIRVHandle() = default;
        SPIRVHandle(const Vector<Uint>& spirv, const String& fileName)
            : SPIRV(spirv), FileName(fileName) {}
        Vector<Uint> SPIRV;
        String FileName;
    };

    class ShaderCompiler
    {
    public:
        static E_NODISCARD SPIRVHandle CompileToSPIRv(const String& name, const String& shaderSource, const ShaderDomain& domain);
        static E_NODISCARD String CrossCompileToGLSL(const SPIRVHandle& spirv);
    };
}
