//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroTimestep.hpp"
#include "Core/Events/ElectroEvent.hpp"

namespace Electro
{
    class Module
    {
    public:
        Module();
        virtual ~Module() = default;

        virtual void Init() {}
        virtual void Shutdown() {}
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& e) {}
    };
}