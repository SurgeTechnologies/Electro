//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Texture.hpp"
#include "Asset/AssetManager.hpp"
#include "Renderer/Renderer.hpp"
#include "Platform/DX11/DX11Texture.hpp"

namespace Electro
{
    Ref<Texture2D> Texture2D::Create(const Texture2DSpecification& spec)
    {
        Ref<Texture2D> result = nullptr;
        switch (Renderer::GetBackend())
        {
            case RendererBackend::DirectX11:
                result = AssetManager::Get<Texture2D>(AssetManager::GetHandle(spec.Path));
                if (!result)
                {
                    result = Ref<DX11Texture2D>::Create(spec);
                    AssetManager::Submit<Texture2D>(result);
                }
        }
        return result;
    }

    Ref<Cubemap> Cubemap::Create(const String& path)
    {
        switch (Renderer::GetBackend())
        {
            case RendererBackend::DirectX11:
                return Ref<DX11Cubemap>::Create(path);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }
}
