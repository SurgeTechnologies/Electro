//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Ref.hpp"
#include "Events/Event.hpp"
#include <glm/glm.hpp>
#include <functional>

namespace Electro
{
    struct WindowProps
    {
        WindowProps() = default;
        WindowProps(const char* title, Uint width, Uint height)
            : Width(width), Height(height), Title(title) {}

        Uint Width = 1440;
        Uint Height = 900;
        const char* Title = "Electro";
    };

    class  Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
        virtual ~Window() = default;

        virtual void Present() = 0;
        virtual void OnUpdate() = 0;

        virtual Uint GetWidth() const = 0;
        virtual Uint GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual void ShowConsole(bool show) = 0;
        virtual void Minimize() = 0;
        virtual void Maximize() = 0;
        virtual bool IsVSync() const = 0;

        virtual void SetTitle(const String& name) = 0;
        virtual const String& GetTitle() const = 0;

        virtual glm::vec2 GetPos() const = 0;
        virtual void SetPos(const glm::vec2& pos) const = 0;

        virtual glm::vec2 GetSize() const = 0;
        virtual void SetSize(const glm::vec2& size) const = 0;

        virtual void* GetNativeWindow() const = 0;
    };
}
