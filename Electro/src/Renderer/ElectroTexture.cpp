//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroTexture.hpp"
#include "Core/ElectroVault.hpp"
#include "ElectroRenderer.hpp"
#include "Platform/DX11/DX11Texture.hpp"

namespace Electro
{
    Ref<Texture2D> Texture2D::Create(Uint width, Uint height)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11Texture2D>::Create(width, height);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const String& path, bool srgb, bool flipped)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11Texture2D>::Create(path, srgb, flipped);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Uint Texture2D::CalculateMipMapCount(Uint width, Uint height)
    {
        Uint levels = 1;
        while ((width | height) >> levels)
            levels++;

        return levels;
    }

    Uint TextureCube::CalculateMipMapCount(Uint width, Uint height)
    {
        return Texture2D::CalculateMipMapCount(width, height);
    }

    Ref<TextureCube> TextureCube::Create(const String& folderpath)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11TextureCube>::Create(folderpath);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}