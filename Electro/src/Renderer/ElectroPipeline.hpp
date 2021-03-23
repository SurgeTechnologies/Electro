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

        //Binds the pipeline ofject to the actual graphics pipeline
        virtual void Bind() const = 0;

        //Binds the specification objects, It binds all the members of PipelineSpecification
        virtual void BindSpecificationObjects() const = 0;

        //Unbinds the pipeline ofject from the actual graphics pipeline, may not work all time
        virtual void Unbind() const = 0;

        //Retrive the PipelineSpecification
        virtual PipelineSpecification& GetSpecification() = 0;

        //Sets the rendering primitive topology
        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;

        //Creates the pipeline, sends the input layout to the GPU and does other good stuff
        static Ref<Pipeline> Create(const PipelineSpecification& spec);
    };
}