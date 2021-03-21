//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "epch.hpp"
#include "DX11Shader.hpp"
#include "DX11Internal.hpp"
#include "Core/ElectroVault.hpp"
#include <d3dcompiler.h>

namespace Electro
{
    static D3D11_SHADER_TYPE ShaderTypeFromString(const String& type)
    {
        if (type == "vertex")
            return D3D11_VERTEX_SHADER;
        if (type == "pixel" || type == "fragment")
            return D3D11_PIXEL_SHADER;

        E_INTERNAL_ASSERT("Unknown shader type!", type);
        return static_cast<D3D11_SHADER_TYPE>(0);
    }

    static String& ShaderVersionFromType(const D3D11_SHADER_TYPE type)
    {
        static String errorString = "No valid conversion found to DX11 shader version from DX11 type ";
        static String vertexVersion = "vs_5_0";
        static String pixelVersion = "ps_5_0";

        switch (type)
        {
            case D3D11_VERTEX_SHADER: return vertexVersion;
            case D3D11_PIXEL_SHADER: return pixelVersion;
        }

        E_INTERNAL_ASSERT("Unknown shader type!", type);
        return errorString;
    }

    DX11Shader::DX11Shader(const String& filepath)
        :mFilepath(filepath)
    {
        mName = Vault::GetNameWithExtension(filepath);
        String source = Vault::ReadFile(filepath);
        mShaderSources = PreProcess(source);
        Compile();

        auto device = DX11Internal::GetDevice();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:
                    DX_CALL(device->CreateVertexShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mVertexShader));
                    break;
                case D3D11_PIXEL_SHADER:
                    DX_CALL(device->CreatePixelShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mPixelShader));
                    break;
            }
        }
    }

    DX11Shader::DX11Shader(const String& source, const char* name)
    {
        mName = name;
        mShaderSources = PreProcess(source);
        Compile();

        auto device = DX11Internal::GetDevice();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:
                    DX_CALL(device->CreateVertexShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mVertexShader));
                    break;
                case D3D11_PIXEL_SHADER:
                    DX_CALL(device->CreatePixelShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mPixelShader));
                    break;
            }
        }
    }

    void DX11Shader::Bind() const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:
                    deviceContext->VSSetShader(mVertexShader, nullptr, 0);
                    break;
                case D3D11_PIXEL_SHADER:
                    deviceContext->PSSetShader(mPixelShader, nullptr, 0);
                    break;
            }
        }
    }

    void DX11Shader::Unbind() const 
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:
                    deviceContext->VSSetShader(nullptr, 0, 0);
                    break;
                case D3D11_PIXEL_SHADER:
                    deviceContext->PSSetShader(nullptr, 0, 0);
                    break;
            }
        }
    }

    void* DX11Shader::GetNativeClass()
    {
        return this;
    }

    std::unordered_map<D3D11_SHADER_TYPE, String> DX11Shader::PreProcess(const String& source)
    {
        std::unordered_map<D3D11_SHADER_TYPE, String> shaderSources;
        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0); //Start of shader type declaration line

        while (pos != String::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
            E_ASSERT(eol != String::npos, "Syntax error");
            size_t being = pos + typeTokenLength + 1; //Start of shader type name(after "#type " keyword)
            String type = source.substr(being, eol - being);
            E_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
            E_ASSERT(nextLinePos != String::npos, "Syntax error");
            pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
            shaderSources[ShaderTypeFromString(type)] = (pos == String::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }
        return shaderSources;
    }

    void DX11Shader::Compile()
    {
        HRESULT result;
        ID3DBlob* errorRaw = nullptr;
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
        #if defined E_DEBUG
            flags |= D3DCOMPILE_DEBUG;
        #endif

        for (auto& kv : mShaderSources)
        {
            D3D11_SHADER_TYPE type = kv.first;
            const String& source = kv.second;

            //https://docs.microsoft.com/en-us/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompile
            result = D3DCompile(source.c_str(), source.size(), NULL, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", ShaderVersionFromType(type).c_str(), flags, 0, &mRawBlobs[type], &errorRaw);

            if (FAILED(result))
            {
                char* errorText = (char*)errorRaw->GetBufferPointer();
                errorText[strlen(errorText) - 1] = '\0';

                ELECTRO_ERROR("%s", errorText);
                errorRaw->Release();
                E_INTERNAL_ASSERT("Shader compilation failure!");
                return; //Useless return
            }
            if (errorRaw)
                errorRaw->Release();
        }
    }

    DX11Shader::~DX11Shader()
    {
        for (auto& kv : mRawBlobs)
            kv.second->Release();
        mRawBlobs.clear();
        if(mVertexShader) mVertexShader->Release();
        if(mPixelShader) mPixelShader->Release();
    }
}