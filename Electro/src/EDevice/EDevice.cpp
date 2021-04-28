//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "EDevice.hpp"
#include "Core/ElectroVault.hpp"
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
    static EDeviceStatus sStatus;
    Ref<VertexBuffer> EDevice::CreateVertexBuffer(Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalVertexBuffers++;
                return Ref<DX11VertexBuffer>::Create(size, layout);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> EDevice::CreateVertexBuffer(void* vertices, Uint size, VertexBufferLayout layout)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalVertexBuffers++;
                return Ref<DX11VertexBuffer>::Create(vertices, size, layout);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<IndexBuffer> EDevice::CreateIndexBuffer(void* indices, Uint count)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalIndexBuffers++;
                return Ref<DX11IndexBuffer>::Create(indices, count);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Framebuffer> EDevice::CreateFramebuffer(const FramebufferSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11: return Ref<DX11Framebuffer>::Create(spec);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Shader> EDevice::CreateShader(const String& filepath)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalShaders++;
                return Ref<DX11Shader>::Create(filepath);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<ConstantBuffer> EDevice::CreateConstantBuffer(Uint size, Uint bindSlot, DataUsage usage)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalConstantBuffers++;
                return Ref<DX11ConstantBuffer>::Create(size, bindSlot, usage);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Pipeline> EDevice::CreatePipeline(const PipelineSpecification& spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalPipelines++;
                return Ref<DX11Pipeline>::Create(spec);
        }
        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> EDevice::CreateTexture2D(Uint width, Uint height)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalTexture2Ds++;
                return Ref<DX11Texture2D>::Create(width, height);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> EDevice::CreateTexture2D(const String& path, bool srgb)
    {
        Ref<Texture2D> result = nullptr;
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                result = Vault::Get<Texture2D>(OS::GetNameWithExtension(path.c_str()));
                if (!result)
                {
                    sStatus.TotalTexture2Ds++;
                    result = Ref<DX11Texture2D>::Create(path, srgb);
                    Vault::Submit<Texture2D>(result);
                }
        }

        return result;
    }

    Ref<Cubemap> EDevice::CreateCubemap(const String& path)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DX11:
                sStatus.TotalCubemaps++;
                return Ref<DX11Cubemap>::Create(path);
        }

        E_INTERNAL_ASSERT("Unknown RendererAPI!");
        return nullptr;
    }

    Ref<EnvironmentMap> EDevice::CreateEnvironmentMap(const String& path)
    {
        Ref<EnvironmentMap> result = Vault::Get<EnvironmentMap>(OS::GetNameWithExtension(path.c_str()));
        if (!result)
        {
            result = Ref<EnvironmentMap>::Create(path);
            Vault::Submit<EnvironmentMap>(result);
        }
        return result;
    }

    Ref<Mesh> EDevice::CreateMesh(const String& path)
    {
        return Ref<Mesh>::Create(path);
    }

    EDeviceStatus EDevice::GetEDeviceStatus()
    {
        return sStatus;
    }
}
