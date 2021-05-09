//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "RenderCommand.hpp"
#include "Platform/DX11/DX11RendererAPI.hpp"

namespace Electro
{
    Scope<RendererAPI> RenderCommand::sRendererAPI = CreateScope<DX11RendererAPI>();
}
