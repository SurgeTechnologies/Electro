//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Texture.hpp"
#include "DX11Internal.hpp"
#include "Core/FileSystem.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Factory.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Renderer/Interface/VertexBuffer.hpp"
#include "Renderer/Interface/IndexBuffer.hpp"
#include "Renderer/Interface/Pipeline.hpp"
#include "Core/Timer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

namespace Electro
{
    DX11Texture2D::DX11Texture2D(Uint width, Uint height)
        : mWidth(width), mHeight(height), mSRV(nullptr), mSRGB(false)
    {
        SetupAssetBase("Built in Texture", AssetType::Texture2D, "Built in Texture");
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

    DX11Texture2D::DX11Texture2D(const String& path, bool srgb)
        : mSRGB(srgb)
    {
        SetupAssetBase(path, AssetType::Texture2D);
        LoadTexture();
    }

    void DX11Texture2D::SetData(void* data, Uint size)
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE ms = {};
        deviceContext->Map(mTexture2D, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, data, size);
        deviceContext->Unmap(mTexture2D, NULL);
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
        if(mTexture2D)
            mTexture2D->Release();
        if(mSRV)
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
    void DX11Texture2D::LoadTexture()
    {
        const char* path = mPathInDisk.c_str();

        // For some reason, *.tga textures are loaded flipped, so flip them vertically so that it loads correctly
        if (FileSystem::GetExtension(path) == ".tga")
            stbi_set_flip_vertically_on_load(true);

        ELECTRO_TRACE("Loading texture from: %s", path);
        int width, height, channels;
        void* data = nullptr;

        if (stbi_is_hdr(path))
        {
            float* pixels = stbi_loadf(path, &width, &height, &channels, 4);
            data = (void*)pixels;
            mIsHDR = true;
        }
        else
        {
            stbi_uc* pixels = stbi_load(path, &width, &height, &channels, 4);
            data = (void*)pixels;
        }

        if (data == nullptr)
        {
            ELECTRO_ERROR("Failed to load image from filepath '%s'!", path);
            stbi_set_flip_vertically_on_load(false);
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
            stbi_set_flip_vertically_on_load(false);
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
        deviceContext->GenerateMips(mSRV);

        free(data);
        stbi_set_flip_vertically_on_load(false);
    }

    /*
        Cubemap
    */

    DX11Cubemap::DX11Cubemap(const String& path)
        : mPath(path), mName(FileSystem::GetNameWithoutExtension(path)), mSRV(nullptr)
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
            case ShaderDomain::None: ELECTRO_WARN("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::Vertex: deviceContext->VSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::Pixel:  deviceContext->PSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::Compute:  deviceContext->CSSetShaderResources(slot, 1, &mNullSRV); break;
        }
    }

    void DX11Cubemap::LoadCubemap()
    {
        //HDR Texture, that will be converted
        Ref<Texture2D> texture = Factory::CreateTexture2D(mPath);

        {
            Timer timer;
            ID3D11Device* device = DX11Internal::GetDevice();
            ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
            Ref<ConstantBuffer> cbuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
            Ref<Shader> shader = AssetManager::Get<Shader>("EquirectangularToCubemap.hlsl");

            Uint width = 512;
            Uint height = 512;

            //Create the TextureCube
            ID3D11Texture2D* tex = nullptr;
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
            DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &tex));

            //Shader Resource view
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = textureDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = -1;
            DX_CALL(device->CreateShaderResourceView(tex, &srvDesc, &mSRV));

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
                ID3D11RenderTargetView* view = nullptr;
                device->CreateRenderTargetView(tex, &renderTargetViewDesc, &rtvs[i]);
            }

            texture->PSBind(0);
            SetViewport(width, height);

            RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLESTRIP);
            for (Uint i = 0; i < 6; i++)
            {
                float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
                deviceContext->ClearRenderTargetView(rtvs[i], col);
                deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);

                shader->Bind();
                cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
                cbuffer->VSBind();
                RenderCommand::Draw(14);
            }
            RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

            deviceContext->GenerateMips(mSRV);

            //Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
            ID3D11RenderTargetView* nullRTV = nullptr;
            deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            //Cleanup
            for (ID3D11RenderTargetView*& rtv : rtvs)
                rtv->Release();
            tex->Release();
            cbuffer.Reset();
            ELECTRO_TRACE("%s to Cubemap conversion took %f seconds", texture->GetName().c_str(), (timer.ElapsedMillis() / 1000));
            texture.Reset();
        }
    }

    RendererID DX11Cubemap::GenIrradianceMap()
    {
        Timer timer;
        ID3D11Device* device = DX11Internal::GetDevice();
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        Ref<ConstantBuffer> cbuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
        Ref<Shader> shader = AssetManager::Get<Shader>("IrradianceConvolution.hlsl");
        Uint width = 32;
        Uint height = 32;


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
        DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &tex));

        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX_CALL(device->CreateShaderResourceView(tex, &srvDesc, &mIrradianceSRV));

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
            device->CreateRenderTargetView(tex, &renderTargetViewDesc, &rtvs[i]);
        }

        SetViewport(width, height);
        deviceContext->PSSetShaderResources(30, 1, &mSRV);

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLESTRIP);
        for (Uint i = 0; i < 6; i++)
        {
            float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            deviceContext->ClearRenderTargetView(rtvs[i], col);
            deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);

            shader->Bind();
            cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
            cbuffer->VSBind();
            RenderCommand::Draw(14);
        }
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

        //Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
        ID3D11RenderTargetView* nullRTV = nullptr;
        deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();

        cbuffer.Reset();
        ELECTRO_TRACE("Irradiance map generation took %f seconds", (timer.ElapsedMillis() / 1000));
        return (RendererID)mIrradianceSRV;
    }

    RendererID DX11Cubemap::GenPreFilter()
    {
        Timer timer;
        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<ConstantBuffer> cbuffer = Factory::CreateConstantBuffer(sizeof(glm::mat4), 0, DataUsage::DYNAMIC);
        Ref<ConstantBuffer> roughnessCBuffer = Factory::CreateConstantBuffer(sizeof(glm::vec4), 4, DataUsage::DYNAMIC);
        Ref<Shader> shader = AssetManager::Get<Shader>("PreFilterConvolution.hlsl");
        Uint width = 128;
        Uint height = 128;

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

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLESTRIP);
        for (Uint mip = 0; mip < maxMipLevels; ++mip)
        {
            Uint mipWidth = static_cast<Uint>(width * std::pow(0.5, mip));
            Uint mipHeight = static_cast<Uint>(height * std::pow(0.5, mip));
            SetViewport(mipWidth, mipHeight);

            glm::vec4 data = { ((float)mip / (float)(maxMipLevels - 1)), 0.0f, 0.0f, 0.0f };
            for (Uint i = 0; i < 6; ++i)
            {
                float col[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
                deviceContext->ClearRenderTargetView(rtvs[mip * 6 + i], col);
                deviceContext->OMSetRenderTargets(1, &rtvs[mip * 6 + i], nullptr);

                shader->Bind();
                cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
                cbuffer->VSBind();
                roughnessCBuffer->SetDynamicData(&data);
                roughnessCBuffer->PSBind();
                deviceContext->PSSetShaderResources(30, 1, &mSRV);
                RenderCommand::Draw(14);
            }
        }
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

        //Bind the null SRV
        deviceContext->PSSetShaderResources(30, 1, &mNullSRV);

        //Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
        ID3D11RenderTargetView* nullRTV = nullptr;
        deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        rtvs.clear();
        ELECTRO_TRACE("Pre Filter map generation took %f seconds", (timer.ElapsedMillis() / 1000));
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
        if(mSRV)
            mSRV->Release();
        if (mIrradianceSRV)
            mIrradianceSRV->Release();
        if (mPreFilterSRV)
            mPreFilterSRV->Release();
    }

    void DX11Cubemap::SetViewport(const Uint& width, const Uint& height)
    {
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        DX11Internal::GetDeviceContext()->RSSetViewports(1, &viewport);
    }
}
