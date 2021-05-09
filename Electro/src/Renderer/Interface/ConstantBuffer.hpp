//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Shader.hpp"

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
        virtual void VSBind() = 0;
        virtual void PSBind() = 0;
        virtual void CSBind() = 0;

        //Returns the data stored in the ConstantBuffer
        virtual void* GetData() = 0;

        //Set the data of this ConstantBuffer via these functions
        virtual void SetStaticData(void* data) = 0;
        virtual void SetDynamicData(void* data) = 0;

        //Returns the Size of the Data stored in the ConstantBuffer
        virtual Uint GetSize() = 0;

        //Returns the native buffer of the underlying API, the underlying API can be DirectX11, OpenGL and maybe Vulkan, who knows!
        virtual RendererID GetNativeBuffer() = 0;

        //Returns the Data Usage of this ConstantBuffer
        virtual DataUsage GetDataUsage() = 0;
    };
}
