//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "DX11Context.hpp"
#include "DX11Internal.hpp"
#include "Core/ElectroApplication.hpp"
#include "DX11Internal.hpp"

namespace Electro
{
    DX11Context::DX11Context(HWND windowHandle)
        :mWindowHandle(windowHandle)
    {
        E_ASSERT(windowHandle, "Window handle is null!");
    }

    void DX11Context::Init()
    {
        DX11Internal::Init(mWindowHandle);
    }

    void DX11Context::SwapBuffers()
    {
        if (Application::Get().GetWindow().IsVSync())
            DX11Internal::GetSwapChain()->Present(1, 0);
        else
            DX11Internal::GetSwapChain()->Present(0, 0);
    }
    DX11Context::~DX11Context()
    {
        DX11Internal::Shutdown();
    }
}