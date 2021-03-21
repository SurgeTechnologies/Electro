//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "Core/ElectroRef.hpp"
#include "Events/ElectroEvent.hpp"
#include <functional>

namespace Electro
{
    struct WindowProps
    {
        WindowProps() = default;
        WindowProps(const char* title, Uint width, Uint height)
            :Title(title), Width(width), Height(height) {}

        Uint Width = 1440;
        Uint Height = 900;
        const char* Title = "Electro";
    };

    class Window : public IElectroRef
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void SetEventCallback(const EventCallbackFn & callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;
        virtual void OnUpdate() = 0;
        virtual void* GetNativeWindow() = 0;
        virtual Uint GetWidth() = 0;
        virtual Uint GetHeight() = 0;
        virtual String& GetTitle() = 0;
        virtual void Present() = 0;

        static Scope<Window> Create(const WindowProps& props = WindowProps());
    };
}