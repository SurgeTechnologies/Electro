//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroVault.hpp"
#include "DX11Texture.hpp"
#include "DX11Internal.hpp"
#include <stb_image.h>

namespace Electro
{
    DX11Texture2D::DX11Texture2D(Uint width, Uint height)
        : mWidth(width), mHeight(height), mFilepath("Built in Texture"), mName("Built in Texture")
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

    DX11Texture2D::DX11Texture2D(const String& path, bool flipped)
        :mFilepath(path), mName(Vault::GetNameWithExtension(mFilepath))
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

    void DX11Texture2D::Reload(bool flip)
    {
        LoadTexture(flip);
    }

    void DX11Texture2D::LoadTexture(bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);

        stbi_uc* data = stbi_load(mFilepath.c_str(), &mWidth, &mHeight, 0, 4);
        if (!data)
            ELECTRO_ERROR("Failed to load image from filepath '%s'!", mFilepath.c_str());

        ID3D11DeviceContext* deviceContext = DX11Internal::GetDeviceContext();

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = mWidth;
        textureDesc.Height = mHeight;
        textureDesc.MipLevels = 0;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //TODO: automate this format
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &mTexture2D)); //Create the Empty texture
        mLoaded = true;

        auto rowPitch = mWidth * 4 * sizeof(unsigned char);
        deviceContext->UpdateSubresource(mTexture2D, 0, 0, data, rowPitch, 0);

        //Create the Shader Resource View
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = -1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(mTexture2D, &srvDesc, &mSRV));
        deviceContext->GenerateMips(mSRV);

        free(data); //Always remember to free the data!
    }

    /*
        Texture Cube
    */

    DX11TextureCube::DX11TextureCube(const String& folderPath)
    {
        Vector<String> paths = Vault::GetAllFilePathsFromParentPath(folderPath);
        mFilePath = folderPath;
        mName = Vault::GetNameWithoutExtension(folderPath);

        for (uint8_t i = 0; i < 6; i++)
            mFaces.emplace_back(paths[i].c_str());

        std::sort(mFaces.begin(), mFaces.end());
        LoadTextureCube(false);
    }

    void DX11TextureCube::Bind(Uint slot, ShaderDomain domain) const
    {
        auto deviceContext = DX11Internal::GetDeviceContext();
        ID3D11SamplerState* sampler = DX11Internal::GetSkyboxSampler();
        deviceContext->PSSetSamplers(1, 1, &sampler);

        switch (domain)
        {
            case ShaderDomain::NONE: ELECTRO_ERROR("Shader domain NONE is given, this is perfectly valid. However, the developer may not want to rely on the NONE."); break;
            case ShaderDomain::VERTEX: deviceContext->VSSetShaderResources(slot, 1, &mSRV); break;
            case ShaderDomain::PIXEL:  deviceContext->PSSetShaderResources(slot, 1, &mSRV); break;
        }

    }

    void DX11TextureCube::Reload(bool flip)
    {
        LoadTextureCube(flip);
    }

    void DX11TextureCube::LoadTextureCube(bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);
        E_ASSERT(mFaces.size() == 6, "TextureCube needs 6 faces!");

        Vector<stbi_uc*> surfaces;
        for (uint8_t i = 0; i < 6; i++)
        {
            int width, height, channels;
            surfaces.emplace_back(stbi_load(mFaces[i].c_str(), &width, &height, &channels, 4));
            mWidth = width;
            mHeight = height;
        }

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = mWidth;
        textureDesc.Height = mHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        D3D11_SUBRESOURCE_DATA datas[6] = {};
        for (uint8_t i = 0; i < 6; i++)
        {
            datas[i].pSysMem = surfaces[i];
            datas[i].SysMemPitch = mWidth * 4 * sizeof(unsigned char);
            datas[i].SysMemSlicePitch = 0;
        }
        mLoaded = true;
        ID3D11Texture2D* tex = nullptr;
        DX_CALL(DX11Internal::GetDevice()->CreateTexture2D(&textureDesc, datas, &tex));

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX_CALL(DX11Internal::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &mSRV));

        //Cleanup
        tex->Release();
        for (uint8_t i = 0; i < 6; i++)
            free(surfaces[i]);
    }

    DX11TextureCube::~DX11TextureCube()
    {
        mSRV->Release();
    }
}