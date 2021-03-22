//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroShader.hpp"

namespace Electro
{
    enum class DataUsage
    {
        DEFAULT = 0,
        DYNAMIC = 1
    };

    struct ConstantBufferDesc
    {
        ConstantBufferDesc() = default;
        Ref<Shader> Shader;
        String Name;
        void* Data;
        Uint Size = 0;
        Uint BindSlot;
        ShaderDomain ShaderDomain = ShaderDomain::VERTEX;
        DataUsage Usage = DataUsage::DYNAMIC;
    };

    class ConstantBuffer : public IElectroRef
    {
    public:
        virtual ~ConstantBuffer() = default;

        //Binds the ConstantBuffer to the pipeline
        virtual void Bind() = 0;

        //Returns the data stored in the ConstantBuffer
        virtual void* GetData() = 0;

        //Set the data of the ConstantBuffer via this function
        virtual void SetData(void* data) = 0;

        //Returns the Size of the Data stored in the ConstantBuffer
        virtual Uint GetSize() = 0;

        //Returns the native buffer of the underlying API, the underlying API can be DirectX11, OpenGL and maybe Vulkan, who knows!
        virtual RendererID GetNativeBuffer() = 0;

        //Gets the domain of the Shader on which this ConstantBuffer is working
        virtual ShaderDomain GetShaderDomain() = 0;

        //Returns the Data Usage of this ConstantBuffer
        virtual DataUsage GetDataUsage() = 0;

        //Creates a ConstantBuffer object, be sure to fill up all the members of ConstantBufferDesc for it to work!
        // const Ref<Shader>& Shader; --> The Shader to work upon
        // const String& Name; --> Name of the ConstantBuffer, must match the Name declared in the Shader
        // void* Data; --> Actual Data (usally a Struct)
        // const Uint Size = 0; --> Size of the Data
        // const Uint BindSlot; --> Bind slot, must match the slot declared in the Shader
        // ShaderDomain ShaderDomain = --> ShaderDomain::VERTEX; The Shader domain
        // DataUsage Usage = DataUsage::DYNAMIC; --> Data Usage
        static Ref<ConstantBuffer> Create(const ConstantBufferDesc& desc);
    };
}