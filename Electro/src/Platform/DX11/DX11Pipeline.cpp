//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Pipeline.hpp"
#include "DX11Shader.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    static DXGI_FORMAT ShaderDataTypeToDirectXBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float:  return DXGI_FORMAT_R32_FLOAT;
            case ShaderDataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
            case ShaderDataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case ShaderDataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case ShaderDataType::Mat3:   return DXGI_FORMAT_R32_FLOAT;
            case ShaderDataType::Mat4:   return DXGI_FORMAT_R32_FLOAT;
            case ShaderDataType::Int:    return DXGI_FORMAT_R32_SINT;
            case ShaderDataType::Int2:   return DXGI_FORMAT_R32G32_SINT;
            case ShaderDataType::Int3:   return DXGI_FORMAT_R32G32B32_SINT;
            case ShaderDataType::Int4:   return DXGI_FORMAT_R32G32B32A32_SINT;
            case ShaderDataType::Bool:   E_INTERNAL_ASSERT("Shader data type bool is not supported!");
        };
        E_INTERNAL_ASSERT("There is no DirectX base type for given shader data type.");
        return DXGI_FORMAT_R32_FLOAT;
    }

    static D3D_PRIMITIVE_TOPOLOGY SpikeTopologyToDX11Topology(PrimitiveTopology topology)
    {
        switch (topology)
        {
            case PrimitiveTopology::UNDEFINED:     return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
            case PrimitiveTopology::POINTLIST:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case PrimitiveTopology::LINELIST:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveTopology::LINESTRIP:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case PrimitiveTopology::TRIANGLELIST:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveTopology::TRIANGLESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        }
        return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    DX11Pipeline::DX11Pipeline(const PipelineSpecification& spec)
        :mSpec(spec)
    {
        auto& elements = mSpec.VertexBuffer->GetLayout().GetElements();
        D3D11_INPUT_ELEMENT_DESC* ied = new D3D11_INPUT_ELEMENT_DESC[elements.size()];

        for (int i = 0; i < elements.size(); i++)
        {
            auto& element = elements[i];
            ied[i] =
            {
                element.Name.c_str(), 0, ShaderDataTypeToDirectXBaseType(element.Type), 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
            };
        }

        DX11Shader* nativeShader = static_cast<DX11Shader*>(mSpec.Shader->GetNativeClass());

        DX_CALL(DX11Internal::GetDevice()->CreateInputLayout(
            ied,
            (UINT)elements.size(),
            static_cast<ID3DBlob*>(nativeShader->GetVSRaw())->GetBufferPointer(),
            static_cast<ID3DBlob*>(nativeShader->GetVSRaw())->GetBufferSize(),
            &mInputLayout));

        delete[] ied;
    }

    DX11Pipeline::~DX11Pipeline()
    {
        if (mInputLayout)
            mInputLayout->Release();
    }

    void DX11Pipeline::Bind() const
    {
        DX11Internal::GetDeviceContext()->IASetPrimitiveTopology(SpikeTopologyToDX11Topology(mPrimitiveTopology));
        DX11Internal::GetDeviceContext()->IASetInputLayout(mInputLayout);
    }

    void DX11Pipeline::BindSpecificationObjects() const
    {
        mSpec.VertexBuffer->Bind();
        mSpec.IndexBuffer->Bind();
        mSpec.Shader->Bind();
    }

    void DX11Pipeline::Unbind() const
    {

    }
}
