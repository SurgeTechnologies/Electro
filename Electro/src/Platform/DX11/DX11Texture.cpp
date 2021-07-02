//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Texture.hpp"
#include "DX11Internal.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Interface/ConstantBuffer.hpp"
#include "Core/Timer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

namespace Electro
{
    DX11Texture2D::DX11Texture2D(const Texture2DSpecification& spec)
        : mSpecification(spec)
    {
        SetupAssetBase(spec.Path, AssetType::Texture2D);
        Load();
    }

    void DX11Texture2D::VSBindAsShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->VSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Texture2D::PSBindAsShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->PSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Texture2D::CSBindAsShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->CSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Texture2D::CSBindAsUnorderedAccess(Uint slot) const
    {
        UINT offset = -1;
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->CSSetUnorderedAccessViews(slot, 1, &mUAV, &offset);
    }

    void DX11Texture2D::BindAsRenderTarget() const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->OMSetRenderTargets(1, &mRTV, mDSV);
    }

    void DX11Texture2D::VSUnbindShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->VSSetShaderResources(slot, 1, &mNullSRV);
    }

    void DX11Texture2D::PSUnbindShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->PSSetShaderResources(slot, 1, &mNullSRV);
    }

    void DX11Texture2D::CSUnbindShaderResource(Uint slot) const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->CSSetShaderResources(slot, 1, &mNullSRV);
    }

    void DX11Texture2D::CSUnbindUnorderedAccess(Uint slot) const
    {
        UINT offset = -1;
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->CSSetUnorderedAccessViews(slot, 1, &mNullUAV, &offset);
    }

    void DX11Texture2D::UnbindAsRenderTarget() const
    {
        ID3D11DeviceContext* deviceContect = DX11Internal::GetDeviceContext();
        deviceContect->OMSetRenderTargets(1, &mNullRTV, mNullDSV);
    }

    namespace Utils
    {
        static DXGI_FORMAT DX11FormatFromElectroTexFormat(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::NONE: return static_cast<DXGI_FORMAT>(-1);
                case TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case TextureFormat::RGBA8UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
                case TextureFormat::R32SINT: return DXGI_FORMAT_R32_SINT;
                case TextureFormat::R32_VOID: return DXGI_FORMAT_R32_TYPELESS;
                case TextureFormat::DEPTH32: return DXGI_FORMAT_D32_FLOAT;
            }
            Log::Error("No Electro Texture Format maps to DXGI_FORMAT(DirectX Format)!");
            return static_cast<DXGI_FORMAT>(-1);
        }
    }

    void DX11Texture2D::Load()
    {
        bool generateMips = HasFlag(TextureFlags::GENERATE_MIPS);
        bool createFronDisk = HasFlag(TextureFlags::CREATE_FROM_DISK);
        bool genShaderResource = HasFlag(TextureFlags::SHADER_RESOURCE);
        bool genDepthView = HasFlag(TextureFlags::DEPTH_STENCIL);
        bool genUnorderedAccess = HasFlag(TextureFlags::COMPUTE_WRITE);
        bool genRenderTarget = HasFlag(TextureFlags::RENDER_TARGET);

        if (createFronDisk)
            E_ASSERT(mSpecification.Width == 0 && mSpecification.Height == 0, "TextureFlags::CREATE_FROM_DISK doesn't support custom Width and Height");

        ID3D11Device* device = DX11Internal::GetDevice();
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();

        ID3D11Texture2D* texture2D = nullptr;

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.ArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

        if (createFronDisk)
            LoadDataAndSetFormat(textureDesc);
        else
            textureDesc.Format = Utils::DX11FormatFromElectroTexFormat(mSpecification.Format);

        textureDesc.Width = mSpecification.Width;
        textureDesc.Height = mSpecification.Height;
        textureDesc.BindFlags = 0;
        mIsHDR ? textureDesc.MipLevels = 1 : textureDesc.MipLevels = 0;

        if (genShaderResource)
            textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        if (genUnorderedAccess)
            textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        if (genRenderTarget || generateMips)
            textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        if (genDepthView)
            textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

        // Create the texture
        DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &texture2D));

        if (createFronDisk)
        {
            UINT rowPitch;
            mIsHDR ? rowPitch = mSpecification.Width * 4 * sizeof(float) : rowPitch = mSpecification.Width * 4 * sizeof(unsigned char);
            deviceContext->UpdateSubresource(texture2D, 0, nullptr, mImageData, rowPitch, 0);
        }

        // Create necessary views
        if (genShaderResource)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = mSpecification.Format == TextureFormat::R32_VOID ? DXGI_FORMAT_R32_FLOAT : textureDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            generateMips ? srvDesc.Texture2D.MipLevels = -1 : srvDesc.Texture2D.MipLevels = 1;

            DX_CALL(device->CreateShaderResourceView(texture2D, &srvDesc, &mSRV));

            if (generateMips)
                deviceContext->GenerateMips(mSRV);
        }

        if (genRenderTarget)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
            renderTargetViewDesc.Format = textureDesc.Format;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;
            DX_CALL(device->CreateRenderTargetView(texture2D, &renderTargetViewDesc, &mRTV));
        }

        if (genUnorderedAccess)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = textureDesc.Format;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = 0;
            DX_CALL(device->CreateUnorderedAccessView(texture2D, &uavDesc, &mUAV));
        }

        if (genDepthView)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
            depthStencilViewDesc.Format = mSpecification.Format == TextureFormat::R32_VOID ? DXGI_FORMAT_D32_FLOAT : textureDesc.Format;
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Texture2D.MipSlice = 0;
            DX_CALL(device->CreateDepthStencilView(texture2D, &depthStencilViewDesc, &mDSV));
        }
        mLoaded = true;
        free(mImageData);
    }

    void DX11Texture2D::LoadDataAndSetFormat(D3D11_TEXTURE2D_DESC& desc)
    {
        int width, height, channels;
        const char* path = mSpecification.Path.c_str();
        if (stbi_is_hdr(path))
        {
            float* pixels = stbi_loadf(path, &width, &height, &channels, 4);
            mImageData = static_cast<void*>(pixels);
            desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            mIsHDR = true;
        }
        else
        {
            stbi_uc* pixels = stbi_load(path, &width, &height, &channels, 4);
            mImageData = static_cast<void*>(pixels);
            if (mSpecification.SRGB)
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            else
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        if (mImageData == nullptr)
        {
            Log::Error("Failed to load image from filepath '{0}'!", path);
            stbi_set_flip_vertically_on_load(false); return;
        }

        mSpecification.Width = width;
        mSpecification.Height = height;
    }

    bool DX11Texture2D::HasFlag(TextureFlags flag)
    {
        return (flag & (mSpecification.Flags));
    }

    DX11Texture2D::~DX11Texture2D()
    {
        if (mSRV) { mSRV->Release(); mSRV = nullptr; }
        if (mRTV) { mRTV->Release(); mRTV = nullptr; }
        if (mDSV) { mDSV->Release(); mDSV = nullptr; }
        if (mUAV) { mUAV->Release(); mUAV = nullptr; }
    }

    DX11Cubemap::DX11Cubemap(const String& path)
        : mPath(path), mName(FileSystem::GetNameWithoutExtension(path))
    {
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
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
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->VSSetSamplers(0, 1, &sampler);
        deviceContext->VSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::PSBind(Uint slot) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->PSSetSamplers(0, 1, &sampler);
        deviceContext->PSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::CSBind(Uint slot) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetComplexSampler();
        deviceContext->CSSetSamplers(0, 1, &sampler);
        deviceContext->CSSetShaderResources(slot, 1, &mSRV);
    }

    void DX11Cubemap::Unbind(Uint slot, ShaderDomain domain) const
    {
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        switch (domain)
        {
        case ShaderDomain::None: Log::Warn("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::Vertex: deviceContext->VSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::Pixel:  deviceContext->PSSetShaderResources(slot, 1, &mNullSRV); break;
            case ShaderDomain::Compute:  deviceContext->CSSetShaderResources(slot, 1, &mNullSRV); break;
        }
    }

    void DX11Cubemap::LoadCubemap()
    {
        // HDR Texture, that will be converted
        Texture2DSpecification textureSpec;
        textureSpec.Path = mPath;
        textureSpec.Flags = TextureFlags::DEFAULT;
        Ref<Texture2D> texture = Texture2D::Create(textureSpec);

        {
            Timer timer;
            ID3D11Device* device = DX11Internal::GetDevice();
            ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
            const Ref<ConstantBuffer>& cbuffer = Renderer::GetConstantBuffer(0);
            Ref<Shader> shader = Renderer::GetShader("EquirectangularToCubemap");

            Uint width = 512;
            Uint height = 512;

            // Create the TextureCube
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

            // Shader Resource view
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
                device->CreateRenderTargetView(tex, &renderTargetViewDesc, &rtvs[i]);
            }

            texture->PSBindAsShaderResource(0);
            shader->Bind();

            DX11Internal::SetViewport({ width, height });
            RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglestrip);

            for (Uint i = 0; i < 6; i++)
            {
                float col[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
                deviceContext->ClearRenderTargetView(rtvs[i], col);
                deviceContext->OMSetRenderTargets(1, &rtvs[i], nullptr);

                cbuffer->SetDynamicData(&mCaptureViewProjection[i]);
                cbuffer->VSBind();
                RenderCommand::Draw(14);
            }
            RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

            deviceContext->GenerateMips(mSRV);

            // Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
            ID3D11RenderTargetView* nullRTV = nullptr;
            deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

            // Cleanup
            for (auto& rtv : rtvs)
                rtv->Release();
            tex->Release();
            Log::Trace("{0} to Cubemap conversion took {1} seconds", FileSystem::GetNameWithExtension(texture->GetSpecification().Path), timer.Elapsed());
            texture.Reset();
        }
    }

    RendererID DX11Cubemap::GenIrradianceMap()
    {
        Timer timer;
        ID3D11Device* device = DX11Internal::GetDevice();
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        const Ref<ConstantBuffer>& cbuffer = Renderer::GetConstantBuffer(0);
        Ref<Shader> shader = Renderer::GetShader("IrradianceConvolution");
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

        DX11Internal::SetViewport({ width, height });
        deviceContext->PSSetShaderResources(30, 1, &mSRV);

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglestrip);
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
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

        //Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
        ID3D11RenderTargetView* nullRTV = nullptr;
        deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();

        Log::Trace("Irradiance map generation took {0} seconds", timer.Elapsed());
        return mIrradianceSRV;
    }

    RendererID DX11Cubemap::GenPreFilter()
    {
        Timer timer;
        ID3D11Device* device = DX11Internal::GetDevice();
        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();
        const Ref<ConstantBuffer>& cbuffer = Renderer::GetConstantBuffer(0);
        const Ref<ConstantBuffer>& roughnessCBuffer = Renderer::GetConstantBuffer(4);
        Ref<Shader> shader = Renderer::GetShader("PreFilterConvolution");
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
        DX_CALL(device->CreateTexture2D(&textureDesc, nullptr, &tex));

        //Shader Resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = -1;
        DX_CALL(device->CreateShaderResourceView(tex, &srvDesc, &mPreFilterSRV));
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
                device->CreateRenderTargetView(tex, &renderTargetViewDesc, &view);
                rtvs.push_back(view);
            }
        }

        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglestrip);
        for (Uint mip = 0; mip < maxMipLevels; ++mip)
        {
            Uint mipWidth = static_cast<Uint>(width * std::pow(0.5, mip));
            Uint mipHeight = static_cast<Uint>(height * std::pow(0.5, mip));
            DX11Internal::SetViewport({ mipWidth, mipHeight });

            glm::vec4 data = { (static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1)), 0.0f, 0.0f, 0.0f };
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
        RenderCommand::SetPrimitiveTopology(PrimitiveTopology::Trianglelist);

        // Bind the null SRV
        deviceContext->PSSetShaderResources(30, 1, &mNullSRV);

        // Bind a null ID3D11RenderTargetView, so that the above ID3D11RenderTargetView's are not written by other stuff
        ID3D11RenderTargetView* nullRTV = nullptr;
        deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

        //Cleanup
        tex->Release();
        for (auto& rtv : rtvs)
            rtv->Release();
        rtvs.clear();
        Log::Trace("Pre Filter map generation took {0} seconds", timer.Elapsed());
        return mPreFilterSRV;
    }

    void DX11Cubemap::BindIrradianceMap(Uint slot) const
    {
        DX11Internal::GetDeviceContext()->PSSetShaderResources(slot, 1, &mIrradianceSRV);
    }

    void DX11Cubemap::BindPreFilterMap(Uint slot) const
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
}
