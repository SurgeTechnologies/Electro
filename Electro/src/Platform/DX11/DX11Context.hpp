//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroGraphicsContext.hpp"
#include "Renderer/ElectroFramebuffer.hpp"
#include <string>
#include <d3d11.h>

namespace Electro
{
    class DX11Context : public GraphicsContext
    {
    public:
        DX11Context(HWND windowHandle);
        virtual ~DX11Context();
        virtual void Init() override;
        virtual void SwapBuffers() override;

    private:
        HWND mWindowHandle;
    };
}
