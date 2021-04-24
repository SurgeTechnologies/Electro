//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/ElectroShader.hpp"

namespace Electro
{
    enum class DataUsage
    {
        DEFAULT = 0,
        DYNAMIC = 1
    };

    class ConstantBuffer : public IElectroRef
    {
    public:
        virtual ~ConstantBuffer() = default;

        //Binds the ConstantBuffer to the pipeline
        virtual void Bind() = 0;

        //Returns the data stored in the ConstantBuffer
        virtual void* GetData() = 0;

        //Set the data of this ConstantBuffer via this function
        virtual void SetData(void* data) = 0;

        //Returns the Size of the Data stored in the ConstantBuffer
        virtual Uint GetSize() = 0;

        //Returns the native buffer of the underlying API, the underlying API can be DirectX11, OpenGL and maybe Vulkan, who knows!
        virtual RendererID GetNativeBuffer() = 0;

        //Gets the domain of the Shader on which this ConstantBuffer is working
        virtual ShaderDomain GetShaderDomain() = 0;

        //Returns the Data Usage of this ConstantBuffer
        virtual DataUsage GetDataUsage() = 0;
    };
}