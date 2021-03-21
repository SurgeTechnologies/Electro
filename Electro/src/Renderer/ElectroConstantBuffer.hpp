//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroShader.hpp"

namespace Electro
{
    enum class DataUsage
    {
        DEFAULT = 0,
        DYNAMIC
    };

    class ConstantBuffer : public IElectroRef
    {
    public:
        virtual ~ConstantBuffer() = default;
        virtual void Bind() = 0;
        virtual void* GetData() = 0;
        virtual void SetData(void* data) = 0;
        virtual Uint GetSize() = 0;

        virtual RendererID GetNativeBuffer() = 0;
        virtual ShaderDomain GetShaderDomain() = 0;
        virtual DataUsage GetDataUsage() = 0;

        static Ref<ConstantBuffer> Create(const Ref<Shader>& shader, const String& name, void* data, const Uint size, const Uint bindSlot, ShaderDomain shaderDomain = ShaderDomain::VERTEX, DataUsage usage = DataUsage::DYNAMIC);
    };
}