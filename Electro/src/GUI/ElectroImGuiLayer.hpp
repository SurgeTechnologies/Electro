//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroLayer.hpp"
#include "Core/Events/ElectroApplicationEvent.hpp"
#include "Core/Events/ElectroKeyEvent.hpp"
#include "Core/Events/ElectroMouseEvent.hpp"

namespace Electro
{
    class  ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& e) override;

        void Begin();
        void End();
        void BlockEvents(bool block) { mBlockEvents = block; }
        void SetDarkThemeColors();
    private:
        bool mBlockEvents = true;
    };

}