//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EGenerator.hpp"
#include "Asset/ElectroAssetManager.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/ElectroRenderer.hpp"
#include "Renderer/ElectroMesh.hpp"
#include "Renderer/ElectroEnvironmentMap.hpp"
#include "Platform/DX11/DX11VertexBuffer.hpp"
#include "Platform/DX11/DX11IndexBuffer.hpp"
#include "Platform/DX11/DX11Framebuffer.hpp"
#include "Platform/DX11/DX11Shader.hpp"
#include "Platform/DX11/DX11ConstantBuffer.hpp"
#include "Platform/DX11/DX11Pipeline.hpp"
#include "Platform/DX11/DX11Texture.hpp"

namespace Electro
{
    Ref<VertexBuffer> EGenerator::CreateVertexBuffer(Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11VertexBuffer>::Create(size, layout);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> EGenerator::CreateVertexBuffer(void* vertices, Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11VertexBuffer>::Create(vertices, size, layout);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<IndexBuffer> EGenerator::CreateIndexBuffer(void* indices, Uint count)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11IndexBuffer>::Create(indices, count);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Framebuffer> EGenerator::CreateFramebuffer(const FramebufferSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11Framebuffer>::Create(spec);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Shader> EGenerator::CreateShader(const String& filepath)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11Shader>::Create(filepath);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<ConstantBuffer> EGenerator::CreateConstantBuffer(Uint size, Uint bindSlot, DataUsage usage)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11ConstantBuffer>::Create(size, bindSlot, usage);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Pipeline> EGenerator::CreatePipeline(const PipelineSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11Pipeline>::Create(spec);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> EGenerator::CreateTexture2D(Uint width, Uint height)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11Texture2D>::Create(width, height);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> EGenerator::CreateTexture2D(const String& path, bool srgb)
    {
        Ref<Texture2D> result = nullptr;
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                result = AssetManager::Get<Texture2D>(OS::GetNameWithExtension(path.c_str()));
                if (!result)
                {
                    result = Ref<DX11Texture2D>::Create(path, srgb);
                    AssetManager::Submit<Texture2D>(result);
                }
        }

        return result;
    }

    Ref<Cubemap> EGenerator::CreateCubemap(const String& path)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                return Ref<DX11Cubemap>::Create(path);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<EnvironmentMap> EGenerator::CreateEnvironmentMap(const String& path)
    {
        Ref<EnvironmentMap> result = AssetManager::Get<EnvironmentMap>(OS::GetNameWithExtension(path.c_str()));
        if (!result)
        {
            result = Ref<EnvironmentMap>::Create(path);
            AssetManager::Submit<EnvironmentMap>(result);
        }
        return result;
    }

    Ref<Mesh> EGenerator::CreateMesh(const String& path)
    {
        return Ref<Mesh>::Create(path);
    }

    Ref<Material> EGenerator::CreateMaterial(const Ref<Shader>& shader, const String& nameInShader)
    {
        return Ref<Material>::Create(shader, nameInShader);
    }
}
