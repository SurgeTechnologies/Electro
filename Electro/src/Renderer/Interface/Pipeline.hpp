//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Renderer/Interface/Shader.hpp"

namespace Electro
{
    class Pipeline : public IElectroRef
    {
    public:
        virtual ~Pipeline() = default;

        // Binds the pipeline object to the actual graphics pipeline
        virtual void Bind() const = 0;

        // Generate the input layout from the shader
        virtual void GenerateInputLayout(const Ref<Shader>& shader) = 0;

        // Gets the stride
        virtual const Uint GetStride() const = 0;

        // Unbinds the pipeline object to the actual graphics pipeline
        virtual void Unbind() const = 0;

        static Ref<Pipeline> Create();
    };
}
