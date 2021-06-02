//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Renderer/Interface/VertexBuffer.hpp"
#include "Renderer/Interface/IndexBuffer.hpp"
#include "Renderer/Interface/Shader.hpp"

namespace Electro
{
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

        //Binds the pipeline object to the actual graphics pipeline
        virtual void Bind() const = 0;

        //Binds the specification objects, It binds all the members of PipelineSpecification
        virtual void BindSpecificationObjects() const = 0;

        //Unbinds the pipeline ofject from the actual graphics pipeline, may not work all time
        virtual void Unbind() const = 0;

        //Retrive the PipelineSpecification
        virtual const PipelineSpecification& GetSpecification() const = 0;
    };
}
