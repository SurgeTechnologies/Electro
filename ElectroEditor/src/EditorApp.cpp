//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#define ELECTRO_ENTRYPOINT
#include <Electro.hpp>
#include "EditorModule.hpp"

namespace Electro
{
    class ElectroEditor : public Application
    {
    public:
        ElectroEditor()
        {
            ApplicationProps appProps;
            appProps.WindowData.Title = "Electro Editor";
            appProps.WindowData.Width = 1280;
            appProps.WindowData.Height = 720;
            appProps.VSync = false;
            appProps.EnableImGui = true;
            appProps.RendererBackend = RendererBackend::DirectX11;
            appProps.ScriptEngineAssemblyPath = "ExampleApp/bin/Release/ExampleApp.dll";
            Init(appProps);
            PushModule(new EditorModule());
        }

        ~ElectroEditor() {}
    };

    Application* CreateApplication()
    {
        return new ElectroEditor();
    }
}
