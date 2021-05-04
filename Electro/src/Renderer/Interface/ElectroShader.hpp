//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroRef.hpp"
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Electro
{
    struct SPIRVHandle;
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
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual const String& GetName() const = 0;
        virtual String GetFilepath() const = 0;

        virtual const String GetSource(const ShaderDomain& domain) const = 0;
        virtual const SPIRVHandle GetSPIRV(const ShaderDomain& domain) const = 0;

        virtual RendererID GetRendererID() const = 0;
        virtual void* GetNativeClass() = 0;
    };
}
