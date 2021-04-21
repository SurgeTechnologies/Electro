//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Scene/ElectroEntity.hpp"

namespace Electro
{
    class MaterialPanel
    {
    public:
        MaterialPanel() = default;
        ~MaterialPanel() = default;
        void Init();
        void OnImGuiRender(bool* show, Entity& selectedEntity);
    };
}