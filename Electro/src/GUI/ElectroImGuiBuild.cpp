//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "epch.hpp"
#include "Renderer/ElectroRendererAPISwitch.hpp"

#ifdef RENDERER_API_DX11
    #include "backends/imgui_impl_dx11.cpp"
#else
    #error No RendererAPI detected
#endif

#include "backends/imgui_impl_win32.cpp" 