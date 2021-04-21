//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Texture.hpp"
#include "DX11Internal.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Core/ElectroVault.hpp"
#include "Renderer/ElectroRenderCommand.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
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

    DX11Texture2D::~DX11Texture2D()
    {
        mTexture2D->Release();
        mSRV->Release();
    }

    void DX11Texture2D::Bind(Uint bindslot, ShaderDomain domain) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();

        ID3D11SamplerState* sampler = DX11Internal::GetCommonSampler();
        deviceContext->PSSetSamplers(0, 1, &sampler);

        switch (domain)
        {
            case ShaderDomain::NONE: ELECTRO_WARN("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::VERTEX: deviceContext->VSSetShaderResources(bindslot, 1, &mSRV); break;
            case ShaderDomain::PIXEL:  deviceContext->PSSetShaderResources(bindslot, 1, &mSRV); break;
        }
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

        free(data); //Always remember to free the data!
        stbi_set_flip_vertically_on_load(false); //Back to default
    }

    /*
        Texture Cube
    */

    DX11TextureCube::DX11TextureCube(const String& path)
        : mPath(path), mName(OS::GetNameWithoutExtension(path)), mSRV(nullptr)
    {
        LoadTextureCube(false);
    }

    void DX11TextureCube::Bind(Uint slot, ShaderDomain domain) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetCommonSampler();
        deviceContext->PSSetSamplers(1, 1, &sampler);

        switch (domain)
        {
            case ShaderDomain::NONE: ELECTRO_WARN("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::VERTEX: deviceContext->VSSetShaderResources(slot, 1, &mSRV); break;
            case ShaderDomain::PIXEL:  deviceContext->PSSetShaderResources(slot, 1, &mSRV); break;
        }
    }

    void DX11TextureCube::LoadTextureCube(bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);
        auto texture = Texture2D::Create(mPath);

        float vertices[] =
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

        Uint indices[] =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<Shader> shader = Vault::Get<Shader>("EquirectangularToCubemap.hlsl");
        Uint width = 512;
        Uint height = 512;
        shader->Bind();

        Ref<ConstantBuffer> cbuffer = ConstantBuffer::Create(sizeof(glm::mat4), 0);

        //Capture projections
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        VertexBufferLayout templayout = { { ShaderDataType::Float3, "POSITION" } };
        Ref<VertexBuffer> tempvertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices), templayout);
        Ref<IndexBuffer> tempindexBuffer = IndexBuffer::Create(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = tempvertexBuffer;
        tempPipelinespec.IndexBuffer = tempindexBuffer;
        tempPipelinespec.Shader = shader;
        auto tempPipeline = Pipeline::Create(tempPipelinespec);

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mSRV));

        //Create the Render target views, for capturing the cube
        Vector<ID3D11RenderTargetView*> rtvs;
        for (Uint i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;
            ID3D11RenderTargetView* view = nullptr;
            DX11Internal::GetDevice()->CreateRenderTargetView(tex, &renderTargetViewDesc, &view);
            rtvs.push_back(view);
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

        texture->Bind(0);
        for (Uint i = 0; i < 6; i++)
        {
            float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            deviceContext->ClearRenderTargetView(rtvs[i], col);
            deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);
            tempPipeline->Bind();
            tempPipeline->BindSpecificationObjects();
            cbuffer->SetData(&captureViews[i]);
            cbuffer->Bind();
            RenderCommand::DrawIndexed(tempPipeline, 36);
        }
        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        texture.Reset();
        stbi_set_flip_vertically_on_load(false); //Back to default
    }

    RendererID DX11TextureCube::GenIrradianceMap()
    {
        auto texture = Texture2D::Create(mPath);

        float vertices[] =
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

        Uint indices[] =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<Shader> shader = Vault::Get<Shader>("IrradianceConvolution.hlsl");
        Uint width = 32;
        Uint height = 32;
        shader->Bind();
        Ref<ConstantBuffer> cbuffer = ConstantBuffer::Create(sizeof(glm::mat4), 0);

        //Capture projections
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        VertexBufferLayout templayout = { { ShaderDataType::Float3, "POSITION" } };
        Ref<VertexBuffer> tempvertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices), templayout);
        Ref<IndexBuffer> tempindexBuffer = IndexBuffer::Create(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = tempvertexBuffer;
        tempPipelinespec.IndexBuffer = tempindexBuffer;
        tempPipelinespec.Shader = shader;
        auto tempPipeline = Pipeline::Create(tempPipelinespec);

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

        //Create the Render target views, for capturing the cube
        Vector<ID3D11RenderTargetView*> rtvs;
        for (Uint i = 0; i < 6; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            renderTargetViewDesc.Texture2DArray.MipSlice = 0;
            renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
            renderTargetViewDesc.Texture2DArray.ArraySize = 1;
            ID3D11RenderTargetView* view = nullptr;
            DX11Internal::GetDevice()->CreateRenderTargetView(tex, &renderTargetViewDesc, &view);
            rtvs.push_back(view);
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
            cbuffer->SetData(&captureViews[i]);
            cbuffer->Bind();
            RenderCommand::DrawIndexed(tempPipeline, 36);
        }
        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        texture.Reset();
        return (RendererID)mIrradianceSRV;
    }

    RendererID DX11TextureCube::GenPreFilter()
    {
        auto texture = Texture2D::Create(mPath);
        float vertices[] =
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

        Uint indices[] =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        auto deviceContext = DX11Internal::GetDeviceContext();
        Ref<Shader> shader = Vault::Get<Shader>("PreFilterConvolution.hlsl");
        Uint width = 128;
        Uint height = 128;
        shader->Bind();
        Ref<ConstantBuffer> cbuffer = ConstantBuffer::Create(sizeof(glm::mat4), 0);
        Ref<ConstantBuffer> roughnessCBuffer = ConstantBuffer::Create(sizeof(glm::vec4), 4, ShaderDomain::PIXEL);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        };

        VertexBufferLayout templayout = { { ShaderDataType::Float3, "POSITION" } };
        Ref<VertexBuffer> tempvertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices), templayout);
        Ref<IndexBuffer> tempindexBuffer = IndexBuffer::Create(indices, static_cast<Uint>(std::size(indices)));
        PipelineSpecification tempPipelinespec;
        tempPipelinespec.VertexBuffer = tempvertexBuffer;
        tempPipelinespec.IndexBuffer = tempindexBuffer;
        tempPipelinespec.Shader = shader;
        auto tempPipeline = Pipeline::Create(tempPipelinespec);

        //Create the TextureCube
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 0;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
            Uint mipWidth = width * std::pow(0.5, mip);
            Uint mipHeight = height * std::pow(0.5, mip);
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
                cbuffer->SetData(&captureViews[i]);
                roughnessCBuffer->SetData(&data);
                deviceContext->PSSetShaderResources(31, 1, &mSRV);
                RenderCommand::DrawIndexed(tempPipeline, 36);
            }
        }

        Vault::Get<Framebuffer>("EditorLayerFramebuffer")->Bind();

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();

        texture.Reset();
        return (RendererID)mIrradianceSRV;
    }

    void DX11TextureCube::BindIrradianceMap(Uint slot)
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mIrradianceSRV);
    }

    void DX11TextureCube::BindPreFilterMap(Uint slot)
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mPreFilterSRV);
    }

    DX11TextureCube::~DX11TextureCube()
    {
        mSRV->Release();
        if (mIrradianceSRV)
            mIrradianceSRV->Release();
        if (mPreFilterSRV)
            mPreFilterSRV->Release();
    }
}
