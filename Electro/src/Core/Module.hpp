//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Timestep.hpp"
#include "Core/Events/Event.hpp"

namespace Electro
{
    class Module
    {
    public:
        Module() = default;
        virtual ~Module() = default;

        virtual void Init() {}
        virtual void Shutdown() {}
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& e) {}
    };
}
