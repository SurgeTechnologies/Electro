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
        virtual const void VSBind() const = 0;
        virtual const void PSBind() const = 0;
        virtual const void CSBind() const = 0;

        //Returns the data stored in the ConstantBuffer
        virtual const void* GetData() const = 0;

        //Set the data of this ConstantBuffer via these functions
        virtual const void SetStaticData(void* data) const = 0;
        virtual const void SetDynamicData(void* data) const = 0;

        //Returns the Size of the Data stored in the ConstantBuffer
        virtual Uint GetSize() = 0;

        //Returns the native buffer of the underlying API, the underlying API can be DirectX11, OpenGL and maybe Vulkan, who knows!
        virtual RendererID GetNativeBuffer() = 0;

        //Returns the Data Usage of this ConstantBuffer
        virtual DataUsage GetDataUsage() = 0;
    };
}
