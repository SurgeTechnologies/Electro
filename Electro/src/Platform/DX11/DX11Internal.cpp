//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Internal.hpp"
#include "Renderer/Factory.hpp"

namespace Electro::DX11Internal
{
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11BlendState* blendState = nullptr;

    ID3D11RasterizerState* normalRasterizerState = nullptr;
    ID3D11RasterizerState* wireframeRasterizerState = nullptr;
    ID3D11RasterizerState* frontCullRasterizerState = nullptr;
    ID3D11RasterizerState* backCullRasterizerState = nullptr;

    ID3D11SamplerState* samplerState = nullptr;
    ID3D11SamplerState* simpleSamplerState = nullptr;
    ID3D11SamplerState* shadowSamplerState = nullptr;

    Ref<Framebuffer> backbuffer = nullptr;
    ID3D11DepthStencilState* lEqualDepthStencilState;
    ID3D11DepthStencilState* lessDepthStencilState;
    Uint width;
    Uint height;

    void Init(HWND hwnd)
    {
        CreateDeviceAndSwapChain(hwnd);
        CreateRasterizerStates();
        CreateBlendStates();
        CreateBackbuffer();
        CreateSamplerStates();
        GenerateVariousDepthStencilStates();
        LogDeviceInfo();
    }

    void Shutdown()
    {
        deviceContext->Release();
        swapChain->Release();
        blendState->Release();
        normalRasterizerState->Release();
        wireframeRasterizerState->Release();
        samplerState->Release();
        simpleSamplerState->Release();
        shadowSamplerState->Release();
        lEqualDepthStencilState->Release();
        lessDepthStencilState->Release();
        device->Release();
    }

    void CreateSamplerStates()
    {
        {
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MinLOD = 0.0f;
            samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
            DX_CALL(device->CreateSamplerState(&samplerDesc, &samplerState));
            deviceContext->PSSetSamplers(0, 1, &samplerState); //Set at slot 0
        }

        {
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            DX_CALL(device->CreateSamplerState(&samplerDesc, &simpleSamplerState));
            deviceContext->PSSetSamplers(1, 1, &simpleSamplerState); //Set at slot 1
        }

        {
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

            samplerDesc.BorderColor[0] = 1.0f;
            samplerDesc.BorderColor[1] = 1.0f;
            samplerDesc.BorderColor[2] = 1.0f;
            samplerDesc.BorderColor[3] = 1.0f;

            DX_CALL(device->CreateSamplerState(&samplerDesc, &shadowSamplerState));
            deviceContext->PSSetSamplers(2, 1, &shadowSamplerState); //Set at slot 2
        }
    }

    void CreateDeviceAndSwapChain(HWND windowHandle)
    {
        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);
        width = clientRect.right - clientRect.left;
        height = clientRect.bottom - clientRect.top;
        E_ASSERT(windowHandle, "Window handle is null!");

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 0;
        sd.Flags = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = windowHandle;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        D3D_FEATURE_LEVEL featureLevels = { D3D_FEATURE_LEVEL_11_0 };
        UINT createDeviceFlags = 0;

#if 1
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        ELECTRO_WARN("[Performance Warning] DirectX 11 Debug layer is enabled, it could impact the performance!");
#endif
        DX_CALL(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &featureLevels, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &deviceContext));
    }

    void CreateBackbuffer()
    {
        FramebufferSpecification backbufferSpec;
        backbufferSpec.SwapChainTarget = true;
        backbufferSpec.Width = width;
        backbufferSpec.Height = height;
        backbufferSpec.Attachments = { FramebufferTextureFormat::RGBA32F, FramebufferTextureFormat::Depth };
        backbuffer = Factory::CreateFramebuffer(backbufferSpec);
    }

    void CreateBlendStates()
    {
        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;

        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&desc, &blendState);
        deviceContext->OMSetBlendState(blendState, nullptr, 0xffffffff);
    }

    void BindBackbuffer() { backbuffer->Bind(); }

    void Resize(Uint width, Uint height)
    {
        backbuffer.Release(); //Terminate the backbuffer
        DX_CALL(swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0)); //Resize the swapchain
        CreateBackbuffer(); //Create the backbuffer
        backbuffer->Resize(width, height);
    }

    void SetViewport(Uint width, Uint height)
    {
        D3D11_VIEWPORT viewport;
        ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        deviceContext->RSSetViewports(1, &viewport);
    }

    ID3D11Device* GetDevice()               { return device;             }
    ID3D11DeviceContext* GetDeviceContext() { return deviceContext;      }
    IDXGISwapChain* GetSwapChain()          { return swapChain;          }
    ID3D11BlendState* GetBlendState()       { return blendState;         }
    ID3D11SamplerState* GetComplexSampler() { return samplerState;       }
    ID3D11SamplerState* GetSimpleSampler()  { return simpleSamplerState; }
    ID3D11SamplerState* GetShadowSampler()  { return shadowSamplerState; }
    Ref<Framebuffer>& GetBackbuffer()       { return backbuffer;         }

    void LogDeviceInfo()
    {
        IDXGIFactory* factory = nullptr;
        IDXGIAdapter* adapter = nullptr;
        DXGI_ADAPTER_DESC adapterDesc;

        CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

        factory->EnumAdapters(0, &adapter);
        adapter->GetDesc(&adapterDesc);

        char videoCardDescription[128];
        String vendor, major, minor, release, build;
        LARGE_INTEGER driverVersion;
        wcstombs_s(NULL, videoCardDescription, 128, adapterDesc.Description, 128);

        if (adapterDesc.VendorId == 0x1002) vendor = "AMD";
        else if (adapterDesc.VendorId == 0x10DE) vendor = "NVIDIA Corporation";
        else if (adapterDesc.VendorId == 0x8086) vendor = "Intel";
        else if (adapterDesc.VendorId == 0x1414) vendor = "Microsoft";
        else                                     vendor = "Unknown vendor!";

        adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion);

        major = std::to_string(HIWORD(driverVersion.HighPart));
        minor = std::to_string(LOWORD(driverVersion.HighPart));
        release = std::to_string(HIWORD(driverVersion.LowPart));
        build = std::to_string(LOWORD(driverVersion.LowPart));

        auto& caps = RendererAPI::GetCapabilities();
        caps.Renderer = videoCardDescription;
        caps.Vendor = vendor;
        caps.Version = String(major.c_str()) + '.' + String(minor.c_str()) + '.' + String(release.c_str()) + '.' + String(build.c_str()) + String(" (SDK version: " + std::to_string(D3D11_SDK_VERSION) + ")");
        caps.MaxTextureUnits = D3D11_STANDARD_VERTEX_ELEMENT_COUNT;
        caps.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
        caps.MaxSamples = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
        ELECTRO_INFO("DirectX Info:");
        ELECTRO_INFO("Vendor: %s", vendor.c_str());
        ELECTRO_INFO("Renderer: %s", videoCardDescription);
        ELECTRO_INFO("Version: %s.%s.%s.%s", major.c_str(), minor.c_str(), release.c_str(), build.c_str());
    }

    void CreateRasterizerStates()
    {
        {   //Normal State
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.CullMode = D3D11_CULL_NONE;
            rasterDesc.FillMode = D3D11_FILL_SOLID;
            rasterDesc.DepthClipEnable = true;
            DX_CALL(device->CreateRasterizerState(&rasterDesc, &normalRasterizerState));
        }

        {   //Front Cull State
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.CullMode = D3D11_CULL_FRONT;
            rasterDesc.FillMode = D3D11_FILL_SOLID;
            rasterDesc.DepthClipEnable = true;
            DX_CALL(device->CreateRasterizerState(&rasterDesc, &frontCullRasterizerState));
        }

        {   //Back Cull State
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.CullMode = D3D11_CULL_BACK;
            rasterDesc.FillMode = D3D11_FILL_SOLID;
            rasterDesc.DepthClipEnable = true;
            DX_CALL(device->CreateRasterizerState(&rasterDesc, &backCullRasterizerState));
        }

        {   // Wireframe state
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.CullMode = D3D11_CULL_NONE;
            rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
            rasterDesc.DepthClipEnable = true;
            DX_CALL(device->CreateRasterizerState(&rasterDesc, &wireframeRasterizerState));
        }

        deviceContext->RSSetState(normalRasterizerState);
    }

    void SetCullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
            case CullMode::Front: deviceContext->RSSetState(frontCullRasterizerState);break;
            case CullMode::Back:  deviceContext->RSSetState(backCullRasterizerState); break;
            case CullMode::None:  deviceContext->RSSetState(normalRasterizerState);   break;
        }
    }
    void BeginWireframe()
    {
        deviceContext->RSSetState(wireframeRasterizerState);
    }

    void EndWireframe()
    {
        deviceContext->RSSetState(normalRasterizerState);
    }

    void GenerateVariousDepthStencilStates()
    {
        //TODO: Come up with a better way of creating states
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = true;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = (D3D11_COMPARISON_FUNC)(1 + (int)DepthTestFunc::Less);
        dsDesc.StencilEnable = true;
        dsDesc.StencilReadMask = 0xFF;
        dsDesc.StencilWriteMask = 0xFF;
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        device->CreateDepthStencilState(&dsDesc, &lessDepthStencilState);

        dsDesc.DepthFunc = (D3D11_COMPARISON_FUNC)(1 + (int)DepthTestFunc::LEqual);
        device->CreateDepthStencilState(&dsDesc, &lEqualDepthStencilState);
    }

    ID3D11DepthStencilState* GetDepthStencilState(DepthTestFunc type)
    {
        switch (type)
        {
            case DepthTestFunc::Never:                                    break;
            case DepthTestFunc::Less:     return lessDepthStencilState;   break;
            case DepthTestFunc::LEqual:   return lEqualDepthStencilState; break;
            case DepthTestFunc::Equal:                                    break;
            case DepthTestFunc::Greater:                                  break;
            case DepthTestFunc::NotEqual:                                 break;
            case DepthTestFunc::GEqual:                                   break;
            case DepthTestFunc::Always:                                   break;
            default: ELECTRO_ERROR("DX11Internal.cpp: No tepth text func matches with the given type! Fix it now!"); break;
        }
        return nullptr;
    }
}

namespace Electro
{
    bool CheckHResult(HRESULT hresult, std::string_view statement, std::string_view file, Uint line)
    {
        if (FAILED(hresult))
        {
            LPSTR buffer;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hresult, 0, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
            ELECTRO_ERROR("[%s(%d)] {%s} failed with error: %s", file.data(), line, statement.data(), buffer);
            return false;
        }
        return true;
    }
}
