//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#define ELECTRO_ENTRYPOINT
#include <Electro.hpp>
#include "ElectroEditorLayer.hpp"

namespace Electro
{
    class ElectroEditor : public Application
    {
    public:
        ElectroEditor()
            :Application("Electro Editor")
        {
            PushLayer(new EditorLayer());
        }

        ~ElectroEditor()
        {
        }
    };

    Application* CreateApplication()
    {
        return new ElectroEditor();
    }   
}