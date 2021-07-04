//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Interface/Renderbuffer.hpp"
#include <d3d11.h>

namespace Electro::DX11Internal
{
    void Init(HWND hwnd);
    void Shutdown();
    void Resize(Uint width, Uint height);
    Viewport GetViewport();
    void SetViewport(Viewport viewport);
    void BindBackbuffer();

    void CreateDeviceAndSwapChain(HWND windowHandle);
    void CreateSamplerStates();
    void CreateBackbuffer();
    void CreateBlendStates();
    void CreateRasterizerStates();
    void LogDeviceInfo();
    void BeginWireframe();
    void EndWireframe();
    void GenerateVariousDepthStencilStates();
    void SetCullMode(CullMode cullMode);
    void EnableDepth();
    void DisableDepth();
    void EnableAdditiveBlending();
    void DisableAdditiveBlending();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();
    IDXGISwapChain* GetSwapChain();
    ID3D11BlendState* GetBlendState();
    ID3D11SamplerState* GetComplexSampler();
    ID3D11SamplerState* GetSimpleSampler();
    ID3D11SamplerState* GetShadowSampler();
    Ref<Renderbuffer>& GetBackbuffer();
}

namespace Electro
{
    bool CheckHResult(HRESULT hresult, std::string_view statement, std::string_view file, Uint line);

    #if E_DEBUG
        #define DX_CALL(func)\
            {\
                HRESULT r = func;\
                CheckHResult((r), #func, __FILE__, __LINE__ );\
                E_ASSERT(SUCCEEDED(r), "DirectX11 Error!");\
            }
    #else
        #define DX_CALL(func) func
    #endif
}
