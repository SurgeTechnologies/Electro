//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Renderer/ElectroVertexBuffer.hpp"
#include "Renderer/ElectroIndexBuffer.hpp"
#include "Renderer/ElectroShader.hpp"

namespace Electro
{
    enum class PrimitiveTopology
    {
        UNDEFINED = 0,
        POINTLIST = 1,
        LINELIST = 2,
        LINESTRIP = 3,
        TRIANGLELIST = 4,
        TRIANGLESTRIP = 5
    };

    struct PipelineSpecification
    {
        Ref<Electro::VertexBuffer> VertexBuffer = nullptr;
        Ref<Electro::Shader> Shader = nullptr;
        Ref<Electro::IndexBuffer> IndexBuffer = nullptr;
    };

    class Pipeline : public IElectroRef
    {
    public:
        virtual ~Pipeline() = default;

        virtual void Bind() const = 0;
        virtual void BindSpecificationObjects() const = 0;
        virtual void Unbind() const = 0;

        virtual PipelineSpecification& GetSpecification() = 0;
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
        static Ref<Pipeline> Create(const PipelineSpecification& spec);
    };
}