//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroShader.hpp"
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3d11shadertracing.h>

namespace Electro
{
    class DX11Shader : public Shader
    {
    public:
        DX11Shader(const String& filepath);
        DX11Shader(const String& source, const char* name);
        virtual ~DX11Shader();
        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void* GetNativeClass() override;
        virtual RendererID GetRendererID() const override { return (RendererID)nullptr; }
        virtual const String& GetName() const override { return mName; }
        virtual String GetFilepath() const override { return mFilepath; };

    public:
        ID3DBlob* GetVSRaw() { return mRawBlobs.at(D3D11_VERTEX_SHADER); }
        ID3DBlob* GetPSRaw() { return mRawBlobs.at(D3D11_PIXEL_SHADER); }

    private:
        std::unordered_map<D3D11_SHADER_TYPE, String> PreProcess(const String& source);
        void Compile();

    private:
        ID3D11VertexShader* mVertexShader = nullptr;
        ID3D11PixelShader*  mPixelShader  = nullptr;
        std::unordered_map<D3D11_SHADER_TYPE, ID3DBlob*> mRawBlobs;
        std::unordered_map<D3D11_SHADER_TYPE, String> mShaderSources;

        String mName; //With Extension
        String mFilepath;
    };
}
