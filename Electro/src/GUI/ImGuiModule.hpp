//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Module.hpp"
#include "Core/Events/ApplicationEvent.hpp"
#include "Core/Events/KeyEvent.hpp"
#include "Core/Events/MouseEvent.hpp"

namespace Electro
{
    class ImGuiModule : public Module
    {
    public:
        ImGuiModule() = default;
        ~ImGuiModule() = default;

        virtual void Init() override;
        virtual void Shutdown() override;
        virtual void OnEvent(Event& e) override;

        void Begin();
        void End();
        void BlockEvents(bool block) { mBlockEvents = block; }
        void SetDarkThemeColors();
    private:
        bool mBlockEvents = true;
    };
}
