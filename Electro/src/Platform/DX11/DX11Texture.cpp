//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Texture.hpp"
#include "DX11Internal.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Core/ElectroVault.hpp"
#include "EDevice/EDevice.hpp"
#include "Renderer/ElectroRenderCommand.hpp"
#include "Renderer/Interface/ElectroConstantBuffer.hpp"
#include "Renderer/Interface/ElectroVertexBuffer.hpp"
#include "Renderer/Interface/ElectroIndexBuffer.hpp"
#include "Renderer/Interface/ElectroPipeline.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

namespace Electro
{
    DX11Texture2D::DX11Texture2D(Uint width, Uint height)
        : mWidth(width), mHeight(height), mFilepath("Built in Texture"), mName("Built in Texture"), mSRV(nullptr), mSRGB(false)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.ArraySize = 1;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.Usage = D3D11_USAGE_DYNAMIC;
        textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Height = mHeight;
        textureDesc.Width = mWidth;
        textureDesc.MipLevels = 1;
        textureDesc.MiscFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;

        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &mTexture2D));
        mLoaded = true;

        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(mTexture2D, nullptr, &mSRV)); //Create the default SRV
    }

    DX11Texture2D::DX11Texture2D(const String& path, bool srgb, bool flipped)
        :mFilepath(path), mName(OS::GetNameWithExtension(mFilepath.c_str())), mSRGB(srgb), mIsFlipped(flipped)
    {
        LoadTexture(flipped);
    }

    void DX11Texture2D::SetData(void* data, Uint size)
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE ms = {};
        deviceContext->Map(mTexture2D, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, data, size);
        deviceContext->Unmap(mTexture2D, NULL);
    }

    void DX11Texture2D::ReloadFlipped()
    {
        if (mIsFlipped)
            mIsFlipped = false;
        else
            mIsFlipped = true;
        LoadTexture(mIsFlipped);
    }

    Uint DX11Texture2D::CalculateMipMapCount(Uint width, Uint height)
    {
        Uint levels = 1;
        while ((width | height) >> levels)
            levels++;

        return levels;
    }

    DX11Texture2D::~DX11Texture2D()
    {
        mTexture2D->Release();
        mSRV->Release();
    }

    void DX11Texture2D::VSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->VSSetSamplers(0, 1, &sampler);
        deviceContext->VSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Texture2D::PSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->PSSetSamplers(0, 1, &sampler);
        deviceContext->PSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Texture2D::CSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->CSSetSamplers(0, 1, &sampler);
        deviceContext->CSSetShaderResources(slot, 1, &mSRV);
    }

    //TODO: Rework this! Have a good way to manage the Formats!
    void DX11Texture2D::LoadTexture(bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);

        int width, height, channels;
        void* data = nullptr;
        if (stbi_is_hdr(mFilepath.c_str()))
        {
            float* pixels = stbi_loadf(mFilepath.c_str(), &width, &height, &channels, 4);
            data = (void*)pixels;
            mIsHDR = true;
        }
        else
        {
            stbi_uc* pixels = stbi_load(mFilepath.c_str(), &width, &height, &channels, 4);
            data = (void*)pixels;
        }

        if (data == nullptr)
        {
            ELECTRO_ERROR("Failed to load image from filepath '%s'!", mFilepath.c_str());
            return;
        }

        mWidth = width;
        mHeight = height;

        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = mWidth;
        textureDesc.Height = mHeight;
        if(!mIsHDR)
            textureDesc.MipLevels = 0;
        else
            textureDesc.MipLevels = 1;

        textureDesc.ArraySize = 1;

        if (mSRGB && mIsHDR)
        {
            ELECTRO_ERROR("Cannot load texture which is both HDR and SRGB! Aborting texture creation...");
            return;
        }

        if (mIsHDR)
            textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        else if(mSRGB)
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        else
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags = 0;
        if(!mIsHDR)
            textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &mTexture2D)); //Create the Empty texture
        mLoaded = true;

        UINT rowPitch;
        if(!mIsHDR)
            rowPitch = mWidth * 4 * sizeof(unsigned char);
        else
            rowPitch = mWidth * 4 * sizeof(float);

        deviceContext->UpdateSubresource(mTexture2D, 0, 0, data, rowPitch, 0);
        //Create the Shader Resource View
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = -1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(mTexture2D, &srvDesc, &mSRV));
        if(!mIsHDR)
            deviceContext->GenerateMips(mSRV);

        free(data);
        stbi_set_flip_vertically_on_load(false);
    }

    /*
        Cubemap
    */

    DX11Cubemap::DX11Cubemap(const String& path)
        : mPath(path), mName(OS::GetNameWithoutExtension(path)), mSRV(nullptr)
    {
        auto captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        mCaptureViewProjection =
        {
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        mCaptureVertices =
        {
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f
        };

        mCaptureIndices =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        LoadCubemap();
    }

    void DX11Cubemap::VSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->VSSetSamplers(0, 1, &sampler);
        deviceContext->VSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::PSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->PSSetSamplers(0, 1, &sampler);
        deviceContext->PSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::CSBind(Uint slot) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->CSSetSamplers(0, 1, &sampler);
        deviceContext->CSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::Unbind(Uint slot, ShaderDomain domain) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        switch (domain)
        {
            case ShaderDomain::NONE: ELECTRO_WARN("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::VERTEX: deviceContext->VSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::PIXEL:  deviceContext->PSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::COMPUTE:  deviceContext->CSSetShaderResources(slot, 1, &mNullSRV); break;
        }
    }

    void DX11Cubemap::LoadCubemap()
    {
        //HDR Texture, that will be converted
        auto texture = EDevice::CreateTexture2D(mPath);

        ID3D11Device* device               = DX11Internal::GetDevice();
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* computeSampler = DX11Internal::GetSimpleSampler();

        Ref<Shader> shader = Vault::Get<Shader>("EquirectangularToCubemap.hlsl");
        Uint width = 1024;
        Uint height = 1024;

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        ID3D11Texture2D* tex = nullptr;
        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &tex));

        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mSRV));

        //Unordered Access View
        ID3D11UnorderedAccessView* const nullUAV[] = { nullptr };
        ID3D11UnorderedAccessView* uav = nullptr;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = textureDesc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.ArraySize = textureDesc.ArraySize;
        DX_CALL(DX11Internal::GetDevice()->CreateUnorderedAccessView(tex, &uavDesc, &uav));

        texture->CSBind(0);
        deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
        deviceContext->CSSetSamplers(1, 1, &computeSampler);
        shader->Bind();
        deviceContext->Dispatch(textureDesc.Width / 32, textureDesc.Height / 32, 6);
        deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

        //Cleanup
        tex->Release();
        uav->Release();
        texture.Reset();
    }

    RendererID DX11Cubemap::GenIrradianceMap()
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<Shader> shader = Vault::Get<Shader>("IrradianceConvolution.hlsl");
        Uint width = 32;
        Uint height = 32;

        Ref<ConstantBuffer> cbuffer = EDevice::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = EDevice::CreateVertexBuffer(mCaptureVertices.data(), sizeof(mCaptureVertices), { { ShaderDataType::Float3, "POSITION" } });
        tempPipelinespec.IndexBuffer  = EDevice::CreateIndexBuffer(mCaptureIndices.data(), static_cast<Uint>(mCaptureIndices.size()));
        tempPipelinespec.Shader = shader;
        auto tempPipeline = EDevice::CreatePipeline(tempPipelinespec);

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        ID3D11Texture2D* tex = nullptr;
        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &tex));

        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mIrradianceSRV));

        //Create the Render target views
        ID3D11RenderTargetView* rtvs[6];
        for (Uint i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;
            DX11Internal::GetDevice()->CreateRenderTargetView(tex, &renderTargetViewDesc, &rtvs[i]);
        }

        //Configure the viewport
        D3D11_VIEWPORT mViewport = {};
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(width);
        mViewport.Height = static_cast<float>(height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &mViewport);
        deviceContext->PSSetShaderResources(31, 1, &mSRV); //31, reserved for irradiance map

        for (Uint i = 0; i < 6; i++)
        {
            float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            deviceContext->ClearRenderTargetView(rtvs[i], col);
            deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);
            tempPipeline->Bind();
            tempPipeline->BindSpecificationObjects();
            cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
            cbuffer->VSBind();
            RenderCommand::DrawIndexed(tempPipeline, 36);
        }

        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        return (RendererID)mIrradianceSRV;
    }

    RendererID DX11Cubemap::GenPreFilter()
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<Shader> shader = Vault::Get<Shader>("PreFilterConvolution.hlsl");
        Uint width = 128;
        Uint height = 128;
        Ref<ConstantBuffer> cbuffer = EDevice::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
        Ref<ConstantBuffer> roughnessCBuffer = EDevice::CreateConstantBuffer(sizeof(glm::vec4), 4, DataUsage::DYNAMIC);

        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = EDevice::CreateVertexBuffer(mCaptureVertices.data(), sizeof(mCaptureVertices), { { ShaderDataType::Float3, "POSITION" } });
        tempPipelinespec.IndexBuffer  = EDevice::CreateIndexBuffer(mCaptureIndices.data(), static_cast<Uint>(mCaptureIndices.size()));
        tempPipelinespec.Shader = shader;
        auto tempPipeline = EDevice::CreatePipeline(tempPipelinespec);

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 0;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
        ID3D11Texture2D* tex = nullptr;
        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &tex));
        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = -1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mPreFilterSRV));
        deviceContext->GenerateMips(mPreFilterSRV);

        Uint maxMipLevels = 5;
        Vector<ID3D11RenderTargetView*> rtvs;
        for (Uint mip = 0; mip < maxMipLevels; ++mip)
        {
            for (Uint i = 0; i < 6; ++i)
            {
                D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
                renderTargetViewDesc.Format = textureDesc.Format;
                renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                renderTargetViewDesc.Texture2DArray.MipSlice = mip;
                renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
                renderTargetViewDesc.Texture2DArray.ArraySize = 1;
                ID3D11RenderTargetView* view = nullptr;
                DX11Internal::GetDevice()->CreateRenderTargetView(tex, &renderTargetViewDesc, &view);
                rtvs.push_back(view);
            }
        }

        for (Uint mip = 0; mip < maxMipLevels; ++mip)
        {
            Uint mipWidth = static_cast<Uint>(width * std::pow(0.5, mip));
            Uint mipHeight = static_cast<Uint>(height * std::pow(0.5, mip));
            D3D11_VIEWPORT mViewport = {};
            mViewport.TopLeftX = 0.0f;
            mViewport.TopLeftY = 0.0f;
            mViewport.Width = static_cast<float>(mipWidth);
            mViewport.Height = static_cast<float>(mipHeight);
            mViewport.MinDepth = 0.0f;
            mViewport.MaxDepth = 1.0f;
            deviceContext->RSSetViewports(1, &mViewport);

            glm::vec4 data = { ((float)mip / (float)(maxMipLevels - 1)), 0.0f, 0.0f, 0.0f };
            for (Uint i = 0; i < 6; ++i)
            {
                float col[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
                deviceContext->ClearRenderTargetView(rtvs[mip * 6 + i], col);
                deviceContext->OMSetRenderTargets(1, &rtvs[mip * 6 + i], nullptr);
                tempPipeline->Bind();
                tempPipeline->BindSpecificationObjects();
                cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
                cbuffer->VSBind();
                roughnessCBuffer->SetDynamicData(&data);
                roughnessCBuffer->PSBind();
                deviceContext->PSSetShaderResources(31, 1, &mSRV);
                RenderCommand::DrawIndexed(tempPipeline, 36);
            }
        }

        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        rtvs.clear();
        return (RendererID)mPreFilterSRV;
    }

    void DX11Cubemap::BindIrradianceMap(Uint slot)
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mIrradianceSRV);
    }

    void DX11Cubemap::BindPreFilterMap(Uint slot)
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mPreFilterSRV);
    }

    Uint DX11Cubemap::CalculateMipMapCount(Uint width, Uint height)
    {
        Uint levels = 1;
        while ((width | height) >> levels)
            levels++;

        return levels;
    }

    DX11Cubemap::~DX11Cubemap()
    {
        mSRV->Release();
        if (mIrradianceSRV)
            mIrradianceSRV->Release();
        if (mPreFilterSRV)
            mPreFilterSRV->Release();
    }
}
