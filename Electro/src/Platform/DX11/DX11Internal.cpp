//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Internal.hpp"

namespace Electro::DX11Internal
{
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;

    ID3D11BlendState* blendState = nullptr;
    ID3D11BlendState* additiveBlendState = nullptr;

    ID3D11RasterizerState* normalRasterizerState = nullptr;
    ID3D11RasterizerState* wireframeRasterizerState = nullptr;
    ID3D11RasterizerState* frontCullRasterizerState = nullptr;
    ID3D11RasterizerState* backCullRasterizerState = nullptr;

    ID3D11SamplerState* samplerState = nullptr;
    ID3D11SamplerState* simpleSamplerState = nullptr;
    ID3D11SamplerState* shadowSamplerState = nullptr;
    ID3D11SamplerState* clampSamplerState = nullptr;

    ID3D11DepthStencilState* normalDepthStencilState;
    ID3D11DepthStencilState* depthStencilDisableState;

    Ref<Renderbuffer> backbuffer = nullptr;
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
        blendState->Release();
        normalRasterizerState->Release();
        wireframeRasterizerState->Release();
        frontCullRasterizerState->Release();
        backCullRasterizerState->Release();

        samplerState->Release();
        simpleSamplerState->Release();
        shadowSamplerState->Release();
        clampSamplerState->Release();

        normalDepthStencilState->Release();
        depthStencilDisableState->Release();

        deviceContext->Release();
        swapChain->Release();
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
            samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
            samplerDesc.BorderColor[0] = 1.0f;
            samplerDesc.BorderColor[1] = 1.0f;
            samplerDesc.BorderColor[2] = 1.0f;
            samplerDesc.BorderColor[3] = 1.0f;

            DX_CALL(device->CreateSamplerState(&samplerDesc, &shadowSamplerState));
            deviceContext->PSSetSamplers(2, 1, &shadowSamplerState);
        }
        {
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
            samplerDesc.BorderColor[0] = 1.0f;
            samplerDesc.BorderColor[1] = 1.0f;
            samplerDesc.BorderColor[2] = 1.0f;
            samplerDesc.BorderColor[3] = 1.0f;

            DX_CALL(device->CreateSamplerState(&samplerDesc, &clampSamplerState));
            deviceContext->PSSetSamplers(3, 1, &clampSamplerState);
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

#if 0
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
        Log::Warn("[Performance Warning] DirectX 11 Debug layer is enabled, it could impact the performance!");
#endif
        DX_CALL(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &featureLevels, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &deviceContext));
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void CreateBackbuffer()
    {
        RenderbufferSpecification backbufferSpec;
        backbufferSpec.SwapChainTarget = true;
        backbufferSpec.Width = width;
        backbufferSpec.Height = height;
        backbufferSpec.Attachments = { RenderBufferTextureFormat::RGBA32F, RenderBufferTextureFormat::DEPTH };
        backbuffer = Renderbuffer::Create(backbufferSpec);
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

        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        device->CreateBlendState(&desc, &additiveBlendState);
    }

    void BindBackbuffer() { backbuffer->Bind(); }

    void Resize(Uint width, Uint height)
    {
        backbuffer.Reset(); // Terminate the backbuffer
        DX_CALL(swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0)); // Resize the swapchain
        CreateBackbuffer(); // Create the backbuffer
        backbuffer->Resize(width, height);
    }

    void SetViewport(Viewport viewport)
    {
        D3D11_VIEWPORT dx11Viewport;
        ZeroMemory(&dx11Viewport, sizeof(D3D11_VIEWPORT));

        dx11Viewport.TopLeftX = viewport.TopLeftX;
        dx11Viewport.TopLeftY = viewport.TopLeftY;
        dx11Viewport.Width = static_cast<float>(viewport.Width);
        dx11Viewport.Height = static_cast<float>(viewport.Height);
        dx11Viewport.MinDepth = viewport.MinDepth;
        dx11Viewport.MaxDepth = viewport.MaxDepth;
        deviceContext->RSSetViewports(1, &dx11Viewport);
    }

    ID3D11Device* GetDevice()               { return device;             }
    ID3D11DeviceContext* GetDeviceContext() { return deviceContext;      }
    IDXGISwapChain* GetSwapChain()          { return swapChain;          }
    ID3D11BlendState* GetBlendState()       { return blendState;         }
    ID3D11SamplerState* GetComplexSampler() { return samplerState;       }
    ID3D11SamplerState* GetSimpleSampler()  { return simpleSamplerState; }
    ID3D11SamplerState* GetShadowSampler()  { return shadowSamplerState; }
    Ref<Renderbuffer>& GetBackbuffer()      { return backbuffer;         }

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

        if (adapterDesc.VendorId == 0x1002) vendor = "Advanced Micro Devices (AMD)";
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
        Log::Info("DirectX Info:");
        Log::Info("Vendor: {0}", vendor.c_str());
        Log::Info("Renderer: {0}", videoCardDescription);
        Log::Info("Version: {0}.{1}.{2}.{3}", major, minor, release, build);
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

        deviceContext->RSSetState(backCullRasterizerState);
    }

    void SetCullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
            case CullMode::FRONT: deviceContext->RSSetState(frontCullRasterizerState);break;
            case CullMode::BACK:  deviceContext->RSSetState(backCullRasterizerState); break;
            case CullMode::NONE:  deviceContext->RSSetState(normalRasterizerState);   break;
        }
    }
    void BeginWireframe()
    {
        deviceContext->RSSetState(wireframeRasterizerState);
    }

    void EndWireframe()
    {
        deviceContext->RSSetState(backCullRasterizerState);
    }

    void GenerateVariousDepthStencilStates()
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

        // Stencil
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;

        // Keep original value on fail
        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

        // Write to the stencil on pass
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Stencil operations if pixel is BackFacing
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        device->CreateDepthStencilState(&depthStencilDesc, &normalDepthStencilState);

        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.StencilReadMask = 0x00;
        depthStencilDesc.StencilWriteMask = 0x00;

        device->CreateDepthStencilState(&depthStencilDesc, &depthStencilDisableState);
    }

    void EnableDepth()
    {
        deviceContext->OMSetDepthStencilState(normalDepthStencilState, 1);
    }

    void DisableDepth()
    {
        deviceContext->OMSetDepthStencilState(depthStencilDisableState, 1);
    }

    void EnableAdditiveBlending()
    {
        deviceContext->OMSetBlendState(additiveBlendState, nullptr, 0xffffffff);
    }

    void DisableAdditiveBlending()
    {
        deviceContext->OMSetBlendState(blendState, nullptr, 0xffffffff);
    }

    Viewport GetViewport()
    {
        Viewport result;
        D3D11_VIEWPORT dx11Viewport;
        UINT numViewport = 1;
        deviceContext->RSGetViewports(&numViewport, &dx11Viewport);

        result.TopLeftX = dx11Viewport.TopLeftX;
        result.TopLeftY = dx11Viewport.TopLeftY;
        result.Width = static_cast<Uint>(dx11Viewport.Width);
        result.Height = static_cast<Uint>(dx11Viewport.Height);
        result.MinDepth = dx11Viewport.MinDepth;
        result.MaxDepth = dx11Viewport.MaxDepth;

        return result;
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
            Log::Error("[{0}({1})] '{2}' failed with error: {3}", file.data(), line, statement.data(), buffer);
            return false;
        }
        return true;
    }
}
