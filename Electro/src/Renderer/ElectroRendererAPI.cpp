//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroRendererAPI.hpp"
#include "ElectroRendererAPISwitch.hpp"

namespace Electro
{
    #ifdef RENDERER_API_OPENGL
        RendererAPI::API RendererAPI::sAPI = RendererAPI::API::OpenGL;
    #elif defined RENDERER_API_DX11
        RendererAPI::API RendererAPI::sAPI = RendererAPI::API::DX11;
    #else
        #error No RendererAPI selected! RendererAPI 'NONE' is currently not supported!
    #endif
}