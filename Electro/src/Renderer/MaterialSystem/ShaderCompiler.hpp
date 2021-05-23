//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Ref.hpp"
#include "Renderer/Interface/Shader.hpp"

namespace Electro
{
    class ShaderReflectionData;
    struct SPIRVHandle
    {
    public:
        SPIRVHandle() = default;
        SPIRVHandle(const Vector<Uint>& spirv, const String& fileName, const ShaderDomain& domain)
            : SPIRV(spirv), FileName(fileName), Domain(domain) {}

        Vector<Uint> SPIRV;
        String FileName;
        ShaderDomain Domain;
    public:
        const bool IsValid() const
        {
            if (SPIRV.empty() || FileName.empty() || Domain == ShaderDomain::None) return false;
            return true;
        }
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
