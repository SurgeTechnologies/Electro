//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Pipeline.hpp"
#include "DX11Shader.hpp"
#include "DX11Internal.hpp"
#include <d3dcompiler.h>

namespace Electro
{
    static Uint DX11ShaderDataTypeSize(DXGI_FORMAT type)
    {
        switch (type)
        {
            case DXGI_FORMAT_R32_FLOAT: return 4;
            case DXGI_FORMAT_R32_SINT:  return 4;
            case DXGI_FORMAT_R32_UINT:  return 4;

            case DXGI_FORMAT_R32G32_FLOAT: return 4 * 2;
            case DXGI_FORMAT_R32G32_SINT:  return 4 * 2;
            case DXGI_FORMAT_R32G32_UINT:  return 4 * 2;

            case DXGI_FORMAT_R32G32B32_FLOAT  : return 4 * 3;
            case DXGI_FORMAT_R32G32B32_SINT:  return 4 * 3;
            case DXGI_FORMAT_R32G32B32_UINT:  return 4 * 3;

            case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4 * 4;
            case DXGI_FORMAT_R32G32B32A32_SINT:  return 4 * 4;
            case DXGI_FORMAT_R32G32B32A32_UINT:  return 4 * 4;
        }
        return 4;
    }

    void DX11Pipeline::GenerateInputLayout(const Ref<Shader>& shader)
    {
        const Ref<DX11Shader>& nativeShader = shader.As<DX11Shader>();
        ID3D11Device* device = DX11Internal::GetDevice();
        ID3DBlob* blob = nativeShader->GetVSRaw();

        // Reflect shader info
        ID3D11ShaderReflection* vertexShaderReflection = nullptr;
        if (FAILED(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&vertexShaderReflection)))
        {
            E_INTERNAL_ASSERT("Cannot reflect DirectX11 Vertex Shader!")
        }

        // Get shader info
        D3D11_SHADER_DESC shaderDesc;
        vertexShaderReflection->GetDesc(&shaderDesc);

        // Read input layout description from shader info
        Vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
        for (Uint i = 0; i < shaderDesc.InputParameters; i++)
        {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

            // Fill out input element desc
            D3D11_INPUT_ELEMENT_DESC elementDesc;
            elementDesc.SemanticName = paramDesc.SemanticName;
            elementDesc.SemanticIndex = paramDesc.SemanticIndex;
            elementDesc.InputSlot = 0;
            elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elementDesc.InstanceDataStepRate = 0;

            // Determine DXGI format
            if (paramDesc.Mask == 1)
            {
                if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
            }
            else if (paramDesc.Mask <= 3)
            {
                if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            }
            else if (paramDesc.Mask <= 7)
            {
                if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            }
            else if (paramDesc.Mask <= 15)
            {
                if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }

            mStride += DX11ShaderDataTypeSize(elementDesc.Format);

            // Save element desc
            inputLayoutDesc.push_back(elementDesc);
        }

        // Try to create Input Layout
        DX_CALL(device->CreateInputLayout(&inputLayoutDesc[0], static_cast<Uint>(inputLayoutDesc.size()), blob->GetBufferPointer(), blob->GetBufferSize(), &mInputLayout));

        // Free allocation shader reflection memory
        vertexShaderReflection->Release();
    }

    void DX11Pipeline::Unbind() const
    {
        DX11Internal::GetDeviceContext()->IASetInputLayout(mNullInputLayout);
    }

    DX11Pipeline::~DX11Pipeline()
    {
        if (mInputLayout)
            mInputLayout->Release();
    }

    void DX11Pipeline::Bind() const
    {
        DX11Internal::GetDeviceContext()->IASetInputLayout(mInputLayout);
    }
}
