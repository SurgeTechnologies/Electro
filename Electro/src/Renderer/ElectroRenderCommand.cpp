//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroRenderCommand.hpp"
#include "ElectroRendererAPISwitch.hpp"
#include "Platform/DX11/DX11RendererAPI.hpp"

namespace Electro
{
    #ifdef RENDERER_API_DX11
        Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<DX11RendererAPI>();
    #else
        #error No RendererAPI selected! RendererAPI 'NONE' is currently not supported!
    #endif
}
