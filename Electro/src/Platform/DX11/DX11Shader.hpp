//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/Interface/Shader.hpp"
#include "Renderer/MaterialSystem/ReflectionData.hpp"

#include <d3d11.h>
#include <d3d11shader.h>
#include <d3d11shadertracing.h>

namespace Electro
{
    class DX11Shader : public Shader
    {
    public:
        DX11Shader(const String& filepath);
        virtual ~DX11Shader();

        virtual void Bind() const override;
        virtual void Reload() override;
        virtual const String GetSource(const ShaderDomain& domain = ShaderDomain::None) const override;
        virtual const SPIRVHandle GetSPIRV(const ShaderDomain& domain) const override;
        virtual const ShaderReflectionData GetReflectionData(const ShaderDomain& domain) const override;
        virtual const String GetName() const { return mName; }
        virtual const String GetPath() const { return mPathInDisk; }

    private:
        ID3DBlob* GetVSRaw() { return mRawBlobs.at(D3D11_VERTEX_SHADER); }
        ID3DBlob* GetPSRaw() { return mRawBlobs.at(D3D11_PIXEL_SHADER); }
        ID3DBlob* GetCSRaw() { return mRawBlobs.at(D3D11_COMPUTE_SHADER); }
        std::unordered_map<D3D11_SHADER_TYPE, String> PreProcess(const String& source);
        void Load();
        void Clear();
        void Compile();

    private:
        ID3D11VertexShader* mVertexShader = nullptr;
        ID3D11PixelShader*  mPixelShader  = nullptr;
        ID3D11ComputeShader* mComputeShader = nullptr;
        String mUnprocessedSource = String();
        String mName = String();
        String mPathInDisk = String();

        std::unordered_map<D3D11_SHADER_TYPE, ID3DBlob*> mRawBlobs;
        std::unordered_map<D3D11_SHADER_TYPE, String> mShaderSources;
        std::unordered_map<D3D11_SHADER_TYPE, SPIRVHandle> mSPIRVs;
        std::unordered_map<ShaderDomain, ShaderReflectionData> mReflectionData;
    private:
        friend class DX11Pipeline;
    };
}
