//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "epch.hpp"
#include "DX11Shader.hpp"
#include "DX11Internal.hpp"
#include "Core/FileSystem.hpp"
#include "Renderer/ReflectionData.hpp"
#include "Renderer/ShaderCompiler.hpp"
#include <d3dcompiler.h>

namespace Electro
{
    namespace Utils
    {
        D3D11_SHADER_TYPE ShaderTypeFromElectroShaderType(ShaderDomain domain)
        {
            switch (domain)
            {
                case ShaderDomain::None:    E_INTERNAL_ASSERT("Shader type NONE is invalid in this context!"); break;
                case ShaderDomain::Vertex:  return D3D11_VERTEX_SHADER;
                case ShaderDomain::Pixel:   return D3D11_PIXEL_SHADER;
                case ShaderDomain::Compute: return D3D11_COMPUTE_SHADER;
                default:
                    E_INTERNAL_ASSERT("Unknown shader type!");
                    return static_cast<D3D11_SHADER_TYPE>(0);
            }

            E_INTERNAL_ASSERT("Unknown shader type!");
            return static_cast<D3D11_SHADER_TYPE>(0);

        }

        static ShaderDomain ElectroShaderTypeFromDX11ShaderType(D3D11_SHADER_TYPE domain)
        {
            switch (domain)
            {
                case D3D11_VERTEX_SHADER:  return ShaderDomain::Vertex;
                case D3D11_PIXEL_SHADER:   return ShaderDomain::Pixel;
                case D3D11_COMPUTE_SHADER: return ShaderDomain::Compute;
                default:
                    E_INTERNAL_ASSERT("Unknown shader type!");
                    return ShaderDomain::None;
            }

            E_INTERNAL_ASSERT("Unknown shader type!");
            return ShaderDomain::None;

        }

        static D3D11_SHADER_TYPE ShaderTypeFromString(const String& type)
        {
            if (type == "vertex")
                return D3D11_VERTEX_SHADER;
            if (type == "pixel" || type == "fragment")
                return D3D11_PIXEL_SHADER;
            if (type == "compute")
                return D3D11_COMPUTE_SHADER;

            E_INTERNAL_ASSERT("Unknown shader type!");
            return static_cast<D3D11_SHADER_TYPE>(0);
        }

        static const char* StringFromShaderType(const D3D11_SHADER_TYPE type)
        {
            switch (type)
            {
                case D3D11_VERTEX_SHADER: return "Vertex";
                case D3D11_PIXEL_SHADER: return "Pixel";
                case D3D11_COMPUTE_SHADER: return "Compute";
                case D3D11_GEOMETRY_SHADER: return "Geometry";
                case D3D11_HULL_SHADER: return "Hull";
                case D3D11_DOMAIN_SHADER: return "Domain";
            }

            E_INTERNAL_ASSERT("Unknown shader type!");
            return "Unknown shader type!";
        }

        static String ShaderVersionFromType(const D3D11_SHADER_TYPE type)
        {
            String errorString = "No valid conversion found to DX11 shader version from DX11 type!";
            String vertexVersion = "vs_5_0";
            String pixelVersion = "ps_5_0";
            String computeVersion = "cs_5_0";

            switch (type)
            {
                case D3D11_VERTEX_SHADER: return vertexVersion;
                case D3D11_PIXEL_SHADER: return pixelVersion;
                case D3D11_COMPUTE_SHADER: return computeVersion;
            }

            E_INTERNAL_ASSERT("Unknown shader type!");
            return errorString;
        }
    }

    DX11Shader::DX11Shader(const String& filepath)
        : mPathInDisk(filepath), mName(FileSystem::GetNameWithExtension(filepath))
    {
        Load();
    }

    void DX11Shader::Bind() const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:  deviceContext->VSSetShader(mVertexShader, nullptr, 0);  break;
                case D3D11_PIXEL_SHADER:   deviceContext->PSSetShader(mPixelShader, nullptr, 0);   break;
                case D3D11_COMPUTE_SHADER: deviceContext->CSSetShader(mComputeShader, nullptr, 0); break;
            }
        }
    }

    void DX11Shader::Reload()
    {
        Load();
        Log::Info("Shader with name {0} was reloaded successfully!", mName);
    }

    const String& DX11Shader::GetSource(const ShaderDomain& domain) const
    {
        E_ASSERT(!mShaderSources.empty(), "Shader source is empty!");

        if (domain == ShaderDomain::None)
            return mUnprocessedSource;

        for (auto& kv : mShaderSources)
            if (kv.first == Utils::ShaderTypeFromElectroShaderType(domain))
                return kv.second; // Return the Source

        return *static_cast<String*>(mNullptr);
    }

    const SPIRVHandle& DX11Shader::GetSPIRV(const ShaderDomain& domain) const
    {
        E_ASSERT(!mSPIRVs.empty(), "SPIRV slots are empty!");

        for (auto& kv : mSPIRVs)
            if (kv.first == Utils::ShaderTypeFromElectroShaderType(domain))
                return kv.second; // Return the SPIRVHandle

        return *static_cast<SPIRVHandle*>(mNullptr);
    }

    const ShaderReflectionData& DX11Shader::GetReflectionData(const ShaderDomain& domain) const
    {
        E_ASSERT(!mReflectionData.empty(), "Reflection data slots are empty!");

        for (auto& kv : mReflectionData)
            if (kv.first == domain)
                return kv.second; // Return the ReflectionData

        return *static_cast<ShaderReflectionData*>(mNullptr);
    }

    std::unordered_map<D3D11_SHADER_TYPE, String> DX11Shader::PreProcess(const String& source)
    {
        std::unordered_map<D3D11_SHADER_TYPE, String> shaderSources;
        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0); //Start of shader type declaration line

        while (pos != String::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line
            E_ASSERT(eol != String::npos, "Syntax error");
            size_t being = pos + typeTokenLength + 1; // Start of shader type name(after "#type " keyword)
            String type = source.substr(being, eol - being);
            E_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
            E_ASSERT(nextLinePos != String::npos, "Syntax error");
            pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
            shaderSources[Utils::ShaderTypeFromString(type)] = (pos == String::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }
        return shaderSources;
    }

    void DX11Shader::Load()
    {
        Clear();

        String source = FileSystem::ReadFile(mPathInDisk);
        if (!source.empty())
            mUnprocessedSource = source;
        else
            Log::Error("Shader source is path {0} is empty!", mPathInDisk);

        mShaderSources = PreProcess(mUnprocessedSource);
        Compile();

        ID3D11Device* device = DX11Internal::GetDevice();
        for (auto& kv : mRawBlobs)
        {
            switch (kv.first)
            {
                case D3D11_VERTEX_SHADER:  DX_CALL(device->CreateVertexShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mVertexShader)); break;
                case D3D11_PIXEL_SHADER:   DX_CALL(device->CreatePixelShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mPixelShader)); break;
                case D3D11_COMPUTE_SHADER: DX_CALL(device->CreateComputeShader(kv.second->GetBufferPointer(), kv.second->GetBufferSize(), NULL, &mComputeShader)); break;
            }
        }

        // Compile to spirv and Reflect the shader
        if (!mComputeShader)
        {
            for (auto& kv : mShaderSources)
            {
                mSPIRVs[kv.first] = ShaderCompiler::CompileToSPIRv(mName, kv.second, Utils::ElectroShaderTypeFromDX11ShaderType(kv.first), true);
                mReflectionData[Utils::ElectroShaderTypeFromDX11ShaderType(kv.first)] = ShaderCompiler::Reflect(mSPIRVs[kv.first], mName);
            }
        }
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

            result = D3DCompile(source.c_str(), source.size(), NULL, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", Utils::ShaderVersionFromType(type).c_str(), flags, 0, &mRawBlobs[type], &errorRaw);

            if (FAILED(result))
            {
                char* errorText = (char*)errorRaw->GetBufferPointer();
                errorText[strlen(errorText) - 1] = '\0';

                Log::Error("{0}", errorText);
                errorRaw->Release();
                Log::Error("{0} shader of {1} failed to compile!", Utils::StringFromShaderType(type), mName);
            }
            if (errorRaw)
                errorRaw->Release();
        }
    }

    void DX11Shader::Clear()
    {
        for (auto& kv : mRawBlobs)
        {
            if (kv.second != nullptr)
            {
                kv.second->Release();
                kv.second = nullptr;
            }
        }
        mRawBlobs.clear();
        mSPIRVs.clear();
        mUnprocessedSource.clear();
        mShaderSources.clear();
        mReflectionData.clear();

        if (mVertexShader)
        {
            mVertexShader->Release();
            mVertexShader = nullptr;
        }

        if (mPixelShader)
        {
            mPixelShader->Release();
            mPixelShader = nullptr;
        }

        if (mComputeShader)
        {
            mComputeShader->Release();
            mComputeShader = nullptr;
        }
    }

    DX11Shader::~DX11Shader()
    {
        for (auto& kv : mRawBlobs)
            kv.second->Release();

        if(mVertexShader)
            mVertexShader->Release();
        if(mPixelShader)
            mPixelShader->Release();
        if(mComputeShader)
            mComputeShader->Release();

        mRawBlobs.clear();
        mSPIRVs.clear();
    }
}
